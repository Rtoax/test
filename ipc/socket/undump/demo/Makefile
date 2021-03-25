UNIX_DOMAIN=rtoax_unsocket_pcap

ALL:
	@echo "UNIX_DOMAIN=${UNIX_DOMAIN}"
	gcc server.c common.c -o server -lm -DUNIX_DOMAIN=\"${UNIX_DOMAIN}\"
	gcc client.c common.c -o client -lm -DUNIX_DOMAIN=\"${UNIX_DOMAIN}\"

clean:
	rm -f server client *.pcap ${UNIX_DOMAIN}
