#!/bin/sh

[ $# -lt 1 ] && { echo "Missing filename" >&2 ; exit 1 ; }
[ $# -gt 1 ] && { echo "Excess arguments" >&2 ; exit 1 ; }
[ -r "$1" ] || { echo "Can't read $1" >&2 ; exit 1 ; }

: ${HOST:=host}
if [ -z "$WHOIS" ]; then
    which "swswhois" > /dev/null 2>&1 && WHOIS="swswhois" || WHOIS="whois"
fi

# read TLDs one per line
parse () {
    local tld rsp whois url urlbase

    [ -d whois ] || mkdir whois

    while read tld ; do
        case "$tld" in
        \#*) # Version
            echo "$tld"
            ;;
        XN--*) # Experimental
            ;;
        *)
            # Get TLD whois
            rsp="whois/$tld.whois"
            if [ ! -e "$rsp" ] && ! $WHOIS -n"whois.iana.org" "$tld" > "$rsp" ; then
                rm -f "$rsp"
                continue
            fi

            # Search response for whois server
            whois=`grep -i "whois.*:" "$rsp" | sed -n "s/[^:]*: *\([^ ]\+\)/\1/p"`
            if [ -n "$whois" ]; then
                # Found the whois server
                echo ".$tld  $whois"
            # Search response for '...registration services: ...'
            elif url=`grep -i -A 1 "registration.*info.*:" $rsp` ; then
                # Extract URL
                urlbase=`echo $url | sed -n "s%.*\(http:\|https:\)//\(www.\|whois.\)*\([^/]*\).*%\1//\2\3%p"`
                case "$urlbase" in
                "") # URL not recognized
                    urlbase=`echo $url | tr "\n" .`
                    echo "#.$tld  $urlbase"
                    continue
                    ;;
                *.*.*) # strip leading http://www.
                    whois="whois.${urlbase#*.}"
                    ;;
                *) # strip leading http://
                    whois="whois.${urlbase#*://}"
                    ;;
                esac

                # See if whois.xxx exists
                if $HOST "$whois" > /dev/null ; then
                    # Found a whois server
                    echo ".$tld  $whois  # $urlbase"
                else
                    echo "#.$tld  $urlbase"
                fi
            else
                # No registration URL
                echo "#.$tld"
            fi
            ;;
        esac
    done
}

parse < $1
