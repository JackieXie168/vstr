#! /bin/sh

if false; then
echo "Do nothing"
elif [ -r ./Documentation ]; then
      doc=./Documentation
elif [ -r ../Documentation ]; then
      doc=../Documentation
else
 echo "No Documentation dir"
 exit 1;
fi

egrep "^Function" $doc/functions.txt | awk '{ print $2 }'