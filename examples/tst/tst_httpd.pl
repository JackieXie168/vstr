#! /usr/bin/perl -w

use strict;
use File::Path;

push @INC, "$ENV{SRCDIR}/tst";
require 'vstr_tst_examples.pl';

sub sub_http_tst
  {
    my $io_r = shift;
    my $io_w = shift;
    my $xtra = shift || {};
    my $sz   = shift;

    my $sock = daemon_connect_tcp();
    my $data = daemon_get_io_r($sock, $io_r);

    $data =~ s/\n/\r\n/g;

    my $output = daemon_io($sock, $data,
			   $xtra->{shutdown_w}, $xtra->{slow_write});

    # Remove date, because that changes each time
    $output =~ s/^(Date:).*$/$1/gm;
    # Remove last-modified = start date for error messages
    $output =~
      s!(HTTP/1[.]1 \s (?:301|400|403|404|405|412|414|416|417|501|505) .*)$ (\n)
	^(Date:)$ (\n)
	^(Server:.*)$ (\n)
	^(Last-Modified:) .*$
	!$1$2$3$4$5$6$7!gmx;
    # Remove last modified for trace ops
    $output =~
      s!^(Last-Modified:).*$ (\n)
        ^(Accept-Ranges:.*)$ (\n)
        ^(Content-Type: \s message/http.*)$
	!$1$2$3$4$5!gmx;
    daemon_put_io_w($io_w, $output);
  }

sub all_vhost_tsts()
  {
    sub_tst(\&sub_http_tst, "ex_httpd");
    sub_tst(\&sub_http_tst, "ex_httpd_errs");
    sub_tst(\&sub_http_tst, "ex_httpd",
	    {shutdown_w => 0});
    sub_tst(\&sub_http_tst, "ex_httpd_errs",
	    {shutdown_w => 0});
    sub_tst(\&sub_http_tst, "ex_httpd",
	    {slow_write => 1});
    sub_tst(\&sub_http_tst, "ex_httpd_errs",
	    {slow_write => 1});
  }

sub all_nonvhost_tsts()
  {
    sub_tst(\&sub_http_tst, "ex_httpd_non-virtual-hosts");
    sub_tst(\&sub_http_tst, "ex_httpd_non-virtual-hosts",
	    {shutdown_w => 0});
    sub_tst(\&sub_http_tst, "ex_httpd_non-virtual-hosts",
	    {slow_write => 1});
  }

if (@ARGV)
  {
    daemon_status(shift);
    if (@ARGV && ($ARGV[0] eq "non-virtual-hosts"))
      {
	all_nonvhost_tsts();
      }
    else
      {
	all_vhost_tsts();
      }
    success();
  }

my $root = "ex_httpd_root";
my $args = $root;

rmtree($root);
mkpath([$root . "/default",
	$root . "/foo.example.com",
	$root . "/foo.example.com:1234"]);

sub make_html
  {
    my $num   = shift;
    my $val   = shift;
    my $fname = shift;

    open(OUT, "> $fname") || failure("open $fname: $!");
    print OUT <<EOL;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2 Final//EN">
<html>
  <head>
    <title>Foo $val</title>
  </head>
  <body>
    <h1>Foo $val</h1>
  </body>
</html>
EOL
    close(OUT) || failure("close");

    my ($a, $b, $c, $d,
	$e, $f, $g, $h,
	$atime, $mtime) = stat("$ENV{SRCDIR}/tst/ex_httpd_tst_1");
    $atime -= ($num * (60 * 60 * 24));
    $mtime -= ($num * (60 * 60 * 24));
    utime $atime, $mtime, $fname;
  }

make_html(1, "root",    "$root/index.html");
make_html(2, "default", "$root/default/index.html");
make_html(3, "norm",    "$root/foo.example.com/index.html");
make_html(4, "port",    "$root/foo.example.com:1234/index.html");
make_html(0, "",        "$root/default/noprivs.html");
chmod(0, "$root/default/noprivs.html");

run_tst("ex_httpd", "ex_httpd_help", "--help");
run_tst("ex_httpd", "ex_httpd_version", "--version");

daemon_init("ex_httpd", $root);
all_vhost_tsts();
daemon_exit("ex_httpd");

daemon_init("ex_httpd", $root, "--mmap");
all_vhost_tsts();
daemon_exit("ex_httpd");

daemon_init("ex_httpd", $root, "--virtual-hosts=false");
all_nonvhost_tsts();
daemon_exit("ex_httpd");

rmtree($root);

success();

END {
  my $save_exit_code = $?;
  daemon_cleanup("ex_httpd");
  $? = $save_exit_code;
}