all:
	make config -C applications/desktop_sniffer/desktop
	make all -C applications/desktop_sniffer/desktop
	make config -C applications/desktop_sniffer/sniff_parse
	make all -C applications/desktop_sniffer/sniff_parse

clean:
	make clean -C applications/desktop_sniffer/desktop
	make clean -C applications/desktop_sniffer/sniff_parse
