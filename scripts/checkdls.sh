#!/bin/bash
# Check github FFAB downloads

 gh api repos/disuye/ffab-releases/releases --jq '.[].assets[] | "\(.name): \(.download_count)"'