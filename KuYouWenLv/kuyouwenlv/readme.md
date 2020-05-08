## kuyouwenlv - 操作和存储路书数据的合约。
### kuyouwenlv合约部署和使用
1. 进入合约文件，编译合约
 `eosio-cpp -I include -o kuyouwenlv.wasm src/kuyouwenlv.cpp --abigen`

2. 创建合约账户
`cleos create account eosio kuyouwenlv EVS5nifJ6sHVSai7LzUK4dQdj3W7vTzgVSnKYZJGDr1nbMfpNK1No`

3. 部署合约
`cleos set contract kuyouwenlv /home/liul/Nutstore/CONTRACTS_DIR/KuYouWenLv/kuyouwenlv -p kuyouwenlv@active`

4. 订单创建
需要user权限去创建订单，且type为0
`cleos push action kuyouwenlv insert '["alice", 0, 2983472986, 10245100]' -p alice@active`
`cleos push action kuyouwenlv insert '["bob", 0, 21348235, 10114500]' -p bob@active`
`cleos push action kuyouwenlv insert '["alice", 0, 364349514, 36542500]' -p alice@active`

5. 订单状态修改
需要kuyouwenlv私钥权限去修改订单状态，且type为1或2，且type为2时系统会返币给user账户
`cleos push action kuyouwenlv updatetype '["alice", 1, 2983472986, 10245100]' -p kuyouwenlv@active`
`cleos push action kuyouwenlv updatetype '["bob", 2, 21348235, 10114500]' -p kuyouwenlv@active`
`cleos push action kuyouwenlv updatetype '["alice", 2, 364349514, 36542500]' -p kuyouwenlv@active`

6. 获取数据
根据主键查询
`cleos get table kuyouwenlv kuyouwenlv kuyouwenlvs --lower 0 --limit 10`
根据用户名获取数据， 用户名是第二索引
`cleos get table kuyouwenlv kuyouwenlv kuyouwenlvs --lower 0 --limit 10 --key-type i64 --index 2`
根据评论的主体（如路书，酒店，机票）类型排序 类型是第三索引
`cleos get table kuyouwenlv kuyouwenlv kuyouwenlvs --lower 0 --limit 10 --key-type i64 --index 3`

7. 删除数据
`cleos push action kuyouwenlv erase '[2983472986]' -p kuyouwenlv@active`

8. 检测是否有某个路书
`cleos push action kuyouwenlv reqkywl '[2983472986]' -p kuyouwenlv@active`