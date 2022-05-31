#!/usr/bin/env sh

set -e

case "$(uname -s)" in
   Darwin)
     brew install coreutils flex
     ;;

   Linux)
     sudo apt-get install libfl-dev flex
     ;;
   *)
     echo 'Other OS'
     ;;
esac
