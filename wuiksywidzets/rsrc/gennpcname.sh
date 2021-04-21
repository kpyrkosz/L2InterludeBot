#!/bin/bash
cut -f-2 raw_npcname | sed -e 's/a,//' -e 's/\\0//' > npcname