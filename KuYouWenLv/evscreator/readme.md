## evscreator合约 - 检查权限合约。
### evscreator合约部署和使用
1. 进入合约文件，编译合约
 `eosio-cpp  -I include -I ../kuyouwenlv/include -o evscreator.wasm src/evscreator.cpp --abigen`

2. 创建合约账户
`cleos create account eosio evscreator EVS5nifJ6sHVSai7LzUK4dQdj3W7vTzgVSnKYZJGDr1nbMfpNK1No`

3. 部署合约
`cleos set contract evscreator /home/liul/Nutstore/CONTRACTS_DIR/KuYouWenLv/evscreator -p evscreator@active`

4. 检查更新合约权限检查
ACTION updatecheck(name user, uint8_t from, uint8_t to);

cleos push action evscreator updatecheck '["evsjerry", '0', '1']' -p evscreator@active

5. 设置部署合约权限
ACTION updatevalue(name user, uint64_t to);

cleos push action evscreator updatevalue '["evsjerry", '1']' -p evscreator@active