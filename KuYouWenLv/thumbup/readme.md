## thumbup合约 - 操作和存储评论数据的合约。
### thumbup合约部署和使用
1. 进入合约文件，编译合约
 `eosio-cpp -I include -I ../kuyouwenlv/include -I ../comment/include -o thumbup.wasm src/thumbup.cpp --abigen`

2. 创建合约账户
`cleos create account eosio thumbup EVS5nifJ6sHVSai7LzUK4dQdj3W7vTzgVSnKYZJGDr1nbMfpNK1No`

3. 部署合约
`cleos set contract thumbup /home/liul/Nutstore/CONTRACTS_DIR/KuYouWenLv/thumbup -p thumbup@active`

4. 授权，可以合约内调用
`cleos set account permission thumbup active '{"threshold": 1,"keys": [{"key": "EVS5nifJ6sHVSai7LzUK4dQdj3W7vTzgVSnKYZJGDr1nbMfpNK1No","weight": 1}],"accounts": [{"permission":{"actor":"thumbup","permission":"eosio.code"},"weight":1}]}' -p thumbup@owner`

5. 点赞上链
`cleos push action thumbup insert '["alice", 1, 21348235, 2423424]' -p thumbup@active`
`cleos push action thumbup insert '["bob", 1, 364349514, 243243]' -p thumbup@active`

6. 获取数据
根据主键获取数据
`cleos get table thumbup thumbup thumbups --lower 0 --limit 10`
根据用户名获取数据， 用户名是第二索引
`cleos get table thumbup thumbup thumbups --lower bob --limit 10 --key-type i64 --index 2`
根据点赞的主体（如路书，酒店，机票）类型排序 类型是第三索引
`cleos get table thumbup thumbup thumbups --lower 0 --limit 10 --key-type i64 --index 3`
根据评论的主体主键排序 主体主键第四索引
`cleos get table thumbup thumbup thumbups --lower 0 --limit 10 --key-type i64 --index 4`

7. 删除数据
`cleos push action thumbup erase '[2423424]' -p thumbup@active`