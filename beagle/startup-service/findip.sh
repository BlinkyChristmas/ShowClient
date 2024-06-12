#!/bin/bash
ip addr show $(ip route | awk '/default/ {print $5 }') | grep "inet" | head -n 1 | awk '/inet/ {print $2}' | cut -d'/' -f1
