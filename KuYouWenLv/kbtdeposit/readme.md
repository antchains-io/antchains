1. 进入合约文件，编译合约
`eosio-cpp -I include -o kbtdeposit.wasm src/kbtdeposit.cpp --abigen`

2. 创建合约账户
`cleos create account eosio kbtdeposit EVS5nifJ6sHVSai7LzUK4dQdj3W7vTzgVSnKYZJGDr1nbMfpNK1No`

3. 部署合约
`cleos set contract kbtdeposit /home/liul/Code/contract/kbtdeposit -p kbtdeposit@active`

4. 插入memo
`cleos push action kbtdeposit insert '{"memo": 1111117}' -p kbtdeposit`

5. 设置精度
`cleos push action kbtdeposit setprecision '{"minimum_in": 100, "minimum_out": 10000}' -p kbtdeposit`

6. 获取数据
根据主键获取数据
`cleos get table kbtdeposit kbtdeposit balances`
`cleos get table kbtdeposit kbtdeposit precision`

7. 删除数据
