/*
    Test OSC 10/11 query support:
    1. Server reply behavior (Terminal::Complete + TerminalColors action + OSC query)
    2. OSC parsing regression (OSC 0/1/2/8/52 still work after multi-digit parser change)
*/

#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>

#include "src/statesync/completeterminal.h"
#include "src/terminal/parseraction.h"
#include "src/terminal/terminal.h"
#include "src/terminal/terminalframebuffer.h"

/* Feed a string through the terminal emulator and return host reply bytes. */
static std::string feed( Terminal::Complete& t, const std::string& s )
{
  return t.act( s );
}

/* Test 1: OSC 10;? and 11;? reply with cached colors. */
static bool test_osc_query_reply( void )
{
  Terminal::Complete term( 80, 24 );

  /* Apply cached colors from client. */
  Parser::TerminalColors colors( "rgb:ffff/ffff/ffff", "rgb:0000/0000/0000" );
  term.act( colors );

  /* Query OSC 10;? (foreground) with ST terminator. */
  std::string reply = feed( term, "\033]10;?\033\\" );
  std::string expected_fg = "\033]10;rgb:ffff/ffff/ffff\033\\";
  if ( reply != expected_fg ) {
    fprintf( stderr, "FAIL test_osc_query_reply: OSC 10 reply mismatch.\n" );
    fprintf( stderr, "  expected: " );
    for ( size_t i = 0; i < expected_fg.size(); i++ )
      fprintf( stderr, "%02x ", (unsigned char)expected_fg[i] );
    fprintf( stderr, "\n  got:      " );
    for ( size_t i = 0; i < reply.size(); i++ )
      fprintf( stderr, "%02x ", (unsigned char)reply[i] );
    fprintf( stderr, "\n" );
    return false;
  }

  /* Query OSC 11;? (background). */
  reply = feed( term, "\033]11;?\033\\" );
  std::string expected_bg = "\033]11;rgb:0000/0000/0000\033\\";
  if ( reply != expected_bg ) {
    fprintf( stderr, "FAIL test_osc_query_reply: OSC 11 reply mismatch.\n" );
    return false;
  }

  fprintf( stderr, "PASS test_osc_query_reply\n" );
  return true;
}

/* Test 2: OSC 10/11 with no cached colors should produce no reply. */
static bool test_osc_query_no_cache( void )
{
  Terminal::Complete term( 80, 24 );

  std::string reply = feed( term, "\033]10;?\033\\" );
  if ( !reply.empty() ) {
    fprintf( stderr, "FAIL test_osc_query_no_cache: unexpected reply for OSC 10.\n" );
    return false;
  }

  reply = feed( term, "\033]11;?\033\\" );
  if ( !reply.empty() ) {
    fprintf( stderr, "FAIL test_osc_query_no_cache: unexpected reply for OSC 11.\n" );
    return false;
  }

  fprintf( stderr, "PASS test_osc_query_no_cache\n" );
  return true;
}

/* Test 3: OSC 10/11 set commands (non-query) should be ignored, not crash. */
static bool test_osc_set_ignored( void )
{
  Terminal::Complete term( 80, 24 );

  Parser::TerminalColors colors( "rgb:ffff/ffff/ffff", "rgb:0000/0000/0000" );
  term.act( colors );

  /* Send a set command (not a query). */
  std::string reply = feed( term, "\033]10;rgb:1111/2222/3333\033\\" );
  if ( !reply.empty() ) {
    fprintf( stderr, "FAIL test_osc_set_ignored: OSC 10 set should not produce a reply.\n" );
    return false;
  }

  fprintf( stderr, "PASS test_osc_set_ignored\n" );
  return true;
}

/* Test 4: BEL-terminated OSC 10;? query. */
static bool test_osc_query_bel( void )
{
  Terminal::Complete term( 80, 24 );

  Parser::TerminalColors colors( "rgb:aaaa/bbbb/cccc", "rgb:1111/2222/3333" );
  term.act( colors );

  /* Query with BEL terminator — reply should also use BEL. */
  std::string reply = feed( term, "\033]11;?\007" );
  std::string expected = "\033]11;rgb:1111/2222/3333\007";
  if ( reply != expected ) {
    fprintf( stderr, "FAIL test_osc_query_bel: OSC 11 reply mismatch.\n" );
    return false;
  }

  fprintf( stderr, "PASS test_osc_query_bel\n" );
  return true;
}

/* Test 5: Regression — OSC 0 (set title) still works. */
static bool test_osc_title_regression( void )
{
  Terminal::Complete term( 80, 24 );

  (void)feed( term, "\033]0;Hello World\033\\" );
  const auto& title = term.get_fb().get_window_title();
  std::wstring expected_str = L"Hello World";
  Terminal::Framebuffer::title_type expected( expected_str.begin(), expected_str.end() );
  if ( title != expected ) {
    fprintf( stderr, "FAIL test_osc_title_regression: title mismatch.\n" );
    return false;
  }

  fprintf( stderr, "PASS test_osc_title_regression\n" );
  return true;
}

/* Test 6: Regression — OSC 2 (set window title, not icon). */
static bool test_osc2_regression( void )
{
  Terminal::Complete term( 80, 24 );

  (void)feed( term, "\033]2;Window Title\007" );
  const auto& title = term.get_fb().get_window_title();
  std::wstring expected_str = L"Window Title";
  Terminal::Framebuffer::title_type expected( expected_str.begin(), expected_str.end() );
  if ( title != expected ) {
    fprintf( stderr, "FAIL test_osc2_regression\n" );
    return false;
  }

  fprintf( stderr, "PASS test_osc2_regression\n" );
  return true;
}

/* Test 7: Regression — OSC 52 (clipboard) still works. */
static bool test_osc52_regression( void )
{
  Terminal::Complete term( 80, 24 );

  (void)feed( term, "\033]52;c;SGVsbG8=\033\\" );
  const auto& clip = term.get_fb().get_clipboard();
  /* OSC 52 stores the raw base64 payload. */
  std::wstring expected_str = L"SGVsbG8=";
  Terminal::Framebuffer::title_type expected( expected_str.begin(), expected_str.end() );
  if ( clip != expected ) {
    fprintf( stderr, "FAIL test_osc52_regression: clipboard mismatch.\n" );
    return false;
  }

  fprintf( stderr, "PASS test_osc52_regression\n" );
  return true;
}

/* Test 8: Regression — OSC 8 (hyperlinks) still works after multi-digit parser change. */
static bool test_osc8_regression( void )
{
  Terminal::Complete term( 80, 24 );

  /* Set a hyperlink via OSC 8. */
  (void)feed( term, "\033]8;;https://example.com\033\\" );
  const auto& link = term.get_fb().ds.get_hyperlink();
  if ( link.empty() ) {
    fprintf( stderr, "FAIL test_osc8_regression: hyperlink not set.\n" );
    return false;
  }

  /* Clear the hyperlink. */
  (void)feed( term, "\033]8;;\033\\" );
  const auto& link2 = term.get_fb().ds.get_hyperlink();
  if ( !link2.empty() ) {
    fprintf( stderr, "FAIL test_osc8_regression: hyperlink not cleared.\n" );
    return false;
  }

  fprintf( stderr, "PASS test_osc8_regression\n" );
  return true;
}

/* Test 9: Malformed query "?junk" should be ignored, not treated as a query. */
static bool test_osc_query_malformed( void )
{
  Terminal::Complete term( 80, 24 );

  Parser::TerminalColors colors( "rgb:ffff/ffff/ffff", "rgb:0000/0000/0000" );
  term.act( colors );

  std::string reply = feed( term, "\033]10;?junk\033\\" );
  if ( !reply.empty() ) {
    fprintf( stderr, "FAIL test_osc_query_malformed: should not reply to '?junk'.\n" );
    return false;
  }

  reply = feed( term, "\033]11;?extra\007" );
  if ( !reply.empty() ) {
    fprintf( stderr, "FAIL test_osc_query_malformed: should not reply to '?extra'.\n" );
    return false;
  }

  fprintf( stderr, "PASS test_osc_query_malformed\n" );
  return true;
}

int main( void )
{
  bool pass = true;

  pass &= test_osc_query_reply();
  pass &= test_osc_query_no_cache();
  pass &= test_osc_set_ignored();
  pass &= test_osc_query_bel();
  pass &= test_osc_title_regression();
  pass &= test_osc2_regression();
  pass &= test_osc52_regression();
  pass &= test_osc8_regression();
  pass &= test_osc_query_malformed();

  if ( pass ) {
    fprintf( stderr, "\nAll OSC 10/11 tests passed.\n" );
    return 0;
  } else {
    fprintf( stderr, "\nSome OSC 10/11 tests FAILED.\n" );
    return 1;
  }
}
