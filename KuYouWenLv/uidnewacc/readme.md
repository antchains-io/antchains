## uidnewacc - 操作和存储路书数据的合约。
### uidnewacc合约部署和使用
1. 进入合约文件，编译合约
 `eosio-cpp -I include -o uidnewacc.wasm src/uidnewacc.cpp --abigen`

2. 创建合约账户
`cleos create account eosio uidnewacc EVS5nifJ6sHVSai7LzUK4dQdj3W7vTzgVSnKYZJGDr1nbMfpNK1No`

3. 部署合约
`cleos set contract uidnewacc /home/liul/Nutstore/CONTRACTS_DIR/KuYouWenLv/uidnewacc -p uidnewacc@active`

4. 插入实名后uid
需要user提交uid给中心服务器，并插入database中
`cleos push action uidnewacc insertuid '["evsfe", b23b7359866a2b725f39fc8b0b0dee11]' -p uidnewacc@active`

5. 创建帐号
当uid在中心数据库中时，才可以创建帐号，创建成功后uid会在中心数据库中被删除
`cleos push action uidnewacc newacc '["evsfe", EVS5WcYrE7iaNWsxictZLBxVTEBFpcwC6gKe4CTR81tdEDPBF5ocZ, b23b7359866a2b725f39fc8b0b0dee11]' -p uidnewacc@active`

6. 生产环境设置
修改evscreator帐号权限为eosio.code
cleos set account permission evscreator active '{"threshold": 1,"keys": [{"key":"EVS5MTwuFHU3odKZSDnJGkxtEKSPJaj7SRAUPPmCQxJHR4bRKc66K", "weight":1}],"accounts": [{"permission":{"actor":"uidnewacc","permission":"eosio.code"},"weight":1}]}' owner -p evscreator@active

7. 数据查询
uid信息查询
`cleos get table uidnewacc uidnewacc uidnewaccs`