#!/bin/bash
OUTPUT=$(/root/bin/findip.sh)  
IFS='.'
read -ra ipparts <<< "$OUTPUT"
retval=0
test=0
if [ -z $ipparts[@] ]; then
  retval=0
else 
  test=${ipparts[0]}
fi

if [ "$test" == "192" ] ; then 
  retval=1
elif [ "$test" == "10" ] ; then 
  retval=1
elif [ "$test" == "172" ] ; then 
  retval=1
fi
exit $retval 
