#!/bin/bash

~/sgame_server/tools/tsf4g/tmeta2tab -B ~/sgame_server/deploy/protocol/db_proto.bin --out_file=./tbl_role.sql 	--meta_name="TBL_ROLE" 	--charset=utf8
~/sgame_server/tools/tsf4g/tmeta2tab -B ~/sgame_server/deploy/protocol/db_proto.bin --out_file=./tbl_account.sql --meta_name="TBL_ACCOUNT" --charset=utf8
