## comment合约 - 操作和存储评论数据的合约。
### comment合约部署和使用
1. 进入合约文件，编译合约
 `eosio-cpp  -I include -I ../kuyouwenlv/include -o comment.wasm src/comment.cpp --abigen`

2. 创建合约账户
`cleos create account eosio comment EVS5nifJ6sHVSai7LzUK4dQdj3W7vTzgVSnKYZJGDr1nbMfpNK1No`

3. 部署合约
`cleos set contract comment /home/liul/Nutstore/CONTRACTS_DIR/KuYouWenLv/comment -p comment@active`

4. 授权，可以合约内调用
`cleos set account permission comment active '{"threshold": 1,"keys": [{"key": "EVS5nifJ6sHVSai7LzUK4dQdj3W7vTzgVSnKYZJGDr1nbMfpNK1No","weight": 1}],"accounts": [{"permission":{"actor":"comment","permission":"eosio.code"},"weight":1}]}' -p comment@owner`

5. 评论上链
`cleos push action comment insert '["alice", 1, 21348235, 2398472, "0123456789012345678901234567890123456789"]' -p comment@active`
`cleos push action comment insert '["bob", 1, 364349514, 3498174, "0123456789012345678901234567890123456789"]' -p comment@active`

6. 获取数据
根据主键获取数据
`cleos get table comment comment comments --lower 0 --limit 10`
根据用户名获取数据， 用户名是第二索引
`cleos get table comment comment comments --lower 0 --limit 10 --key-type i64 --index 2`
根据评论的主体（如路书，酒店，机票）类型排序 类型是第三索引
`cleos get table comment comment comments --lower 0 --limit 10 --key-type i64 --index 3`
根据评论的主体主键排序 主体主键第四索引
`cleos get table comment comment comments --lower 0 --limit 10 --key-type i64 --index 4`

7. 删除数据
`cleos push action comment erase '[2398472]' -p comment@active`
`cleos push action comment erase '[3498174]' -p comment@active`
