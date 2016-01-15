Maza Developer Notes
========================

| | |
| ------------------- | ------------------------------------ |
| Algorithm           | SHA256                               |
| Block Reward        | 1000 MAZA                             |
| Block Time          | 2 minutes                            |
| Reward Halving      | 950,000 blocks                       |
| Min Block Rewad     | 1 MAZA                                |
| Difficulty Retarget | every 1 block using DarkGravityWave3 |

Total coin mined: 2.398 billion MAZA + yearly inflation

2.398 billion coins will be mined in approx. 36yrs

---
## Main Network

### Ports
| service/protocol | port  |
| ---------------- | ----- |
| RPC              | 12835 |
| P2P              | 12832 |
| p2pool           | 14476 |


### Genesis Block
```javascript
{
    "hash" : "00000c7c73d8ce604178dae13f0fc6ec0be3275614366d44b1b4b5c6e238c60c",
    "size" : 285,
    "height" : 0,
    "version" : 1,
    "merkleroot" : "62d496378e5834989dd9594cfc168dbb76f84a39bbda18286cddc7d1d1589f4f",
    "tx" : [
        "62d496378e5834989dd9594cfc168dbb76f84a39bbda18286cddc7d1d1589f4f"
    ],
    "time" : 1390747675, // Sun, 26 Jan 2014 14:47:55 GMT
    "nonce" : 2091390249,
    "bits" : "1e0ffff0",
    "difficulty" : 0.00024414,
    "chainwork" : "0000000000000000000000000000000000000000000000000000000000100010",
    "nextblockhash" : "000000003302fe58f139f1d45f3a0a67601d39e63b82bc4918f48b8cd5df6ab0"
}
```

packetMagic: 0xF8B503DF
```C
pchMessageStart[0] = 0xf8;
pchMessageStart[1] = 0xb5;
pchMessageStart[2] = 0x03;
pchMessageStart[3] = 0xdf;
```

---
## Test Network

### Ports
| service/protocol | port  |
| ---------------- | ----- |
| RPC              | 11835 |
| P2P              | 11444 |

```C
pchMessageStart[0] = 0x05;
pchMessageStart[1] = 0xfe;
pchMessageStart[2] = 0xa9;
pchMessageStart[3] = 0x01;
```
