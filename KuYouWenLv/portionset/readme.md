## portionset - 设置订单参数返现比例数据的合约。
### portionset合约部署和使用
1. 进入合约文件，编译合约
 `eosio-cpp -I include -o portionset.wasm src/portionset.cpp --abigen`

2. 创建合约账户
`cleos create account eosio portionset EVS5nifJ6sHVSai7LzUK4dQdj3W7vTzgVSnKYZJGDr1nbMfpNK1No`

3. 部署合约
`cleos set contract portionset /home/jerry/evs/eosis-clwv/KuYouWenLv/portionset -p portionset@active`

4. 设置订单参数返现比例数据
`cleos push action portionset setportion '[100]' -p portionset@active`

cleos get table portionset portionset portion
{
  "rows": [{
      "order_proportion": 100
    }
  ],
  "more": false
}
