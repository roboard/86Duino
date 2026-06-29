#!/bin/sh
# Parse IANA IPv4 Address Space Registry
# http://www.iana.org/assignments/ipv4-address-space/ipv4-address-space.txt

[ $# -lt 1 ] && { echo "Missing filename" >&2 ; exit 1 ; }
[ $# -gt 1 ] && { echo "Excess arguments" >&2 ; exit 1 ; }
[ -r "$1" ] || { echo "Can't read $1" >&2 ; exit 1 ; }

# Parse: prefix whois status
parse() {
    local addr whois status mask
    while read addr whois status ; do
        #echo "'$addr' '$whois' '$status'" >&2
        mask=${addr##*/}
        addr=${addr%%/*}
        addr=${addr#0}
        addr=${addr#0}
        addr="$addr.0.0.0/$mask"

        if [ -z "$status" ]; then
            status="$whois"
            whois=
        fi

        case "$status" in
        ALLOCATED|LEGACY)
            if [ "$whois" != "" ]; then
                echo "$addr $whois"
            else
                echo "#$addr $status"
            fi
            ;;
        UNALLOCATED|RESERVED)
            echo "#$addr $status"
            ;;
        *)
            #echo "'$addr' '$whois' '$status'" >&2
            ;;
        esac
    done
}

# Split into prefix whois status
cut -c 1-9,66-83,84-93 $1 | parse
