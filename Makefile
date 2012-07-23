all:
# July 22, 2012 -- At this time, the applications/desktop_sniffer/desktop project has not been ported to Windows
	make config -C applications/desktop_sniffer/sniff_parse
	make all -C applications/desktop_sniffer/sniff_parse
	
# July 22, 2012 -- If using Windows, comment out the two lines below.
	make config -C applications/desktop_sniffer/desktop
	make all -C applications/desktop_sniffer/desktop

clean:
	make clean -C applications/desktop_sniffer/desktop
	make clean -C applications/desktop_sniffer/sniff_parse
