#!/bin/bash
grep 'Seed:' items | cut -f2 | sort | uniq | cut -d' ' -f2- | while read -r line; do grep -P "\t$line\t" items | cut -f-2; done > Manor_crops
