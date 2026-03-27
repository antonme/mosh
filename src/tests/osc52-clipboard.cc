/*
    Mosh: the mobile shell
    Copyright 2026 Anton Volnuhin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    In addition, as a special exception, the copyright holders give
    permission to link the code of portions of this program with the
    OpenSSL library under certain conditions as described in each
    individual source file, and distribute linked combinations including
    the two.

    You must obey the GNU General Public License in all respects for all
    of the code used other than OpenSSL. If you modify file(s) with this
    exception, you may extend this exception to your version of the
    file(s), but you are not obligated to do so. If you do not wish to do
    so, delete this exception statement from your version. If you delete
    this exception statement from all source files in the program, then
    also delete it here.
*/

#include <cstdio>
#include <cstdlib>
#include <string>

#include "src/terminal/parseraction.h"
#include "src/terminal/terminaldispatcher.h"
#include "src/terminal/terminalframebuffer.h"

using Terminal::Dispatcher;
using Terminal::Framebuffer;

namespace {

std::string to_string( const Framebuffer::title_type& title )
{
  std::string out;
  out.reserve( title.size() );
  for ( Framebuffer::title_type::const_iterator i = title.begin(); i != title.end(); i++ ) {
    out.push_back( static_cast<char>( *i ) );
  }
  return out;
}

std::string run_osc52( const std::string& payload )
{
  Dispatcher dispatcher;
  Framebuffer fb( 80, 24 );
  Parser::OSC_Start osc_start;
  Parser::OSC_Put osc_put;
  Parser::OSC_End osc_end;

  dispatcher.OSC_start( &osc_start );
  for ( std::string::const_iterator i = payload.begin(); i != payload.end(); i++ ) {
    osc_put.ch = static_cast<unsigned char>( *i );
    osc_put.char_present = true;
    dispatcher.OSC_put( &osc_put );
  }
  dispatcher.OSC_dispatch( &osc_end, &fb );

  return to_string( fb.get_clipboard() );
}

void expect_clipboard( const char* label, const std::string& payload, const std::string& expected )
{
  const std::string actual = run_osc52( payload );
  if ( actual != expected ) {
    fprintf( stderr, "%s: expected [%s], got [%s]\n", label, expected.c_str(), actual.c_str() );
    exit( 1 );
  }
}

} // namespace

int main( void )
{
  expect_clipboard( "exact-c", "52;c;YmxhYmxh", "YmxhYmxh" );
  expect_clipboard( "multi-target", "52;cp;YmxhYmxh", "YmxhYmxh" );
  expect_clipboard( "empty-options", "52;;YmxhYmxh", "YmxhYmxh" );
  return 0;
}
