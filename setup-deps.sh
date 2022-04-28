#!/usr/bin/env sh

set -e

case "$(uname -s)" in
   Darwin)
     brew install flex
     ;;

   Linux)
     apt-get install libfl-dev flex
     ;;
   *)
     echo 'Other OS'
     ;;
esac
