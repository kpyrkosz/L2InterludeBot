#!/bin/bash
cut -f1,3 skills_raw | sort -n | uniq | sed -e 's/a,//' -e 's/\\0$//'> skills_clean
