#!/bin/bash
git shortlog --summary --numbered --email | cut -f 2- > AUTHORS
