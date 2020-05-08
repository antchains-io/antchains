## precisionset - 设置订单参数精度数据的合约。
### precisionset合约部署和使用
1. 进入合约文件，编译合约
 `eosio-cpp -I include -o precisionset.wasm src/precisionset.cpp --abigen`

2. 创建合约账户
`cleos create account eosio precisionset EVS5nifJ6sHVSai7LzUK4dQdj3W7vTzgVSnKYZJGDr1nbMfpNK1No`

3. 部署合约
`cleos set contract precisionset /home/jerry/evs/eosis-clwv/KuYouWenLv/precisionset -p precisionset@active`

4. 设置订单参数精度数据
`cleos push action precisionset setprecision '[100]' -p precisionset@active`

cleos get table precisionset precisionset precision
{
  "rows": [{
      "order_precision": 100
    }
  ],
  "more": false
}
