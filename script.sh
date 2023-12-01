#!/bin/bash
regg1="^[A-Z][a-zA-Z0-9, ]*[\.$\!$\?$]"
regg2=",[]*si[]"
regg3="[]*si[],"
count=0
if [ "$#" -ne 1 ];
    then
    echo "Numar de argumente mai mare de 1"
    exit 1
fi
caracter=$1

while read linie
do
    if echo $linie | grep -E "$regg1" | grep -v "$regg2" | grep -v "$regg3" | grep "$caracter"
    then
        ((count++))        
    fi
done
echo "$count"
exit 0