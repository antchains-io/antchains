## extractset - 设置运营账户提取时间数据的合约。
### extractset合约部署和使用
1. 进入合约文件，编译合约
 `eosio-cpp -I include -o extractset.wasm src/extractset.cpp --abigen`

2. 创建合约账户
`cleos create account eosio extractset EVS5nifJ6sHVSai7LzUK4dQdj3W7vTzgVSnKYZJGDr1nbMfpNK1No`

3. 部署合约
`cleos set contract extractset /home/jerry/evs/eosis-clwv/KuYouWenLv/extractset -p extractset@active`

4. 设置运营账户提取时间数据
`cleos push action extractset setpextract '[5]' -p extractset@active`

cleos get table extractset extractset extracttime
{
  "rows": [{
      "extract_time_hour": 5
    }
  ],
  "more": false
}
