#!/bin/bash
for MANOR in {Gludio,Aden,Goddard,Oren,Dion,Giran,Innadril,Schuttgart,Rune}
	do (grep 'Seed:.*'$MANOR items | cut -f-2 | sed 's/Seed: //')>Seeds_$MANOR
done