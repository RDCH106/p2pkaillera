This is a list of Kaillera vulnerabilities discovered in a number of clients and servers.

# Open Kaillera Vulnerabilities #


1.  The following functions in kaillera\_ui.cpp use insecure string copying resulting in buffer overflows:

kaillera\_gdebug, kaillera\_core\_debug, kaillera\_ui\_motd, kaillera\_error\_callback,kaillera\_ui\_debug, and kaillera\_outpf

# Ownaclient Vulnerabilites #

1.  Function re\_append in uihlp.h of Ownaclient introduces remote buffer overflow in handling large amounts of text: can be exploited client-side for DoS effect in Emulinker and EMX  with:(/msg, /setemu, gamechat) (no reliable exploitation thanks to /GS).

-Fixed in upload 2 of Ownaclient.

2.  Ownaclient inherits [Issue #1](https://code.google.com/p/p2pkaillera/issues/detail?id=#1) from Open Kaillera (because its a modification of that client).  In v18 kaillera\_ui\_motd is patched, however all other similar functions are left vulnerable.

-Patched in upload 1 of Ownaclient.

3.  Two off-by-one overflows exist in Ownaclient v17 relating to username and quit message (not serious because they are local).

-Patched in v18 of Owna's client after I reported it to him.

4.  Crash involved locally in sending long private message.

-Patched in v18 of Owna's client after I reported it to him.

# Kaillera.com/Anti3d Client #

1.  There is a buffer overflow in handling of long usernames which can be triggered via various methods in the Kaillera protocol.

2.  Denial of service if null byte is used in a username (tested with chat).

3.  Local Denial of Service (paste long string into Custom IP input box)

# Supraclient #

1.  Local buffer overflow in IP and Quit Message input boxes.

2. Multiple buffer overflows in handling of overlong messages coming from Server (MOTD, Quit message, and many others)

3.  DoS with long username in Server Status packet.

4.  Extremely long emulator name causes access violation when hosting game.

# Kaillera 0.86 Server Vulnerabilities #

1.  Denial of Service Attack if malformed packet sent after "hello" packet.

No patch available.

2.  Buffer overflow found by Luigi Auriemma: http://aluigi.org/adv/kailleraex-adv.txt

Patch available at: http://aluigi.org/patches/kailleraexfix.lpatch


# Exploits #

This script here on Exploit-db demonstrates several buffer overflow vulnerabilities in Open Kaillera, the original Kaillera client, and Supraclient: http://www.exploit-db.com/exploits/17460/

This Metasploit module will exploit the 0.86 server DoS vulnerability:

http://metasploit.com/svn/framework3/trunk/modules/auxiliary/dos/windows/games/kaillera.rb

# Caveats #

In terms of real-world exploitability, most of these issues require a rogue / malicious Kaillera server.

However, there are several issues which can be triggered client-side (even with servers like Emulinker and EMX which are supposed to protect against "hackers").

## Patches ##

This Google Code website offers patches for Ownaclient v18 (latest) and Open Kaillera (latest compiled version).

No known patches for Supraclient or Kaillera.com/Anti3d (mostly because there is no source code).  They are old and full of holes, I advise you to upgrade your client if you run these.