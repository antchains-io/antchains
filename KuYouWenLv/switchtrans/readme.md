## switchtrans - 设置全局转账开关参数的合约。
### switchtrans合约部署和使用
1. 进入合约文件，编译合约
 `eosio-cpp -I include -o switchtrans.wasm src/switchtrans.cpp --abigen`

2. 创建合约账户
`cleos create account eosio switchtrans EVS5nifJ6sHVSai7LzUK4dQdj3W7vTzgVSnKYZJGDr1nbMfpNK1No`

3. 部署合约
`cleos set contract switchtrans /home/jerry/evs/eosis-clwv/KuYouWenLv/switchtrans -p switchtrans@active`

4. 设置全局转账开关参数
`cleos push action switchtrans settrans '[1]' -p switchtrans@active`

cleos get table switchtrans switchtrans swstate
{
  "rows": [{
      "switch_state": 1
    }
  ],
  "more": false
}