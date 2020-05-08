## register - 操作和存储路书数据的合约。
### register合约部署和使用
1. 进入合约文件，编译合约
 `eosio-cpp -I include -o registration.wasm src/registration.cpp --abigen`

2. 创建合约账户
`cleos create account eosio registration EVS5nifJ6sHVSai7LzUK4dQdj3W7vTzgVSnKYZJGDr1nbMfpNK1No`

3. 部署合约
`cleos set contract registration /home/liul/Nutstore/CONTRACTS_DIR/KuYouWenLv/registration -p registration@active`

4. 授权，可以合约内调用
`cleos set account permission registration active '{"threshold": 1,"keys": [{"key": "EVS5GGdCa3nUU3DF2UnJqPJuHh5LUC1yTf8nXkGpwDtgniD9sM3zj","weight": 1}],"accounts": [{"permission":{"actor":"registration","permission":"eosio.code"},"weight":1}]}' -p registration@owner`

5. 通过合约注册账户
`cleos push action registration insert '["registration", "liufasong", "EVS5GGdCa3nUU3DF2UnJqPJuHh5LUC1yTf8nXkGpwDtgniD9sM3zj", "KDceC"]' -p registration@active`

6. 获取数据
根据主键查询
`cleos get table registration registration registration --lower "" --limit 2`
根据推广的人获取数据， 推广人是第二索引
`cleos get table registration registration registration --lower "" --limit 2 --key-type i64 --index 2`