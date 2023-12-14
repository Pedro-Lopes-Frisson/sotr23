# SOTR - Project 02

## Authors

|     Name     | Nmec  |
| :----------: | :---: |
| Gonçalo Leal | 98008 |
| Pedro Lopes  | 97827 |

## Possible actions

- Set the value of each one of the digital outputs (individually)
- Set the value of all the digital outputs (all at once, atomic operation)
- Read the value of the digital inputs
- Read the newest value of temperature
- Read the past 20 values of temperature
- Read the max and min temperature
- Reset the temperature history (including max and min)


## Some Commands

| Command     | CheckSum | Example Response                                                    | Description                                               |
| :---------- | :------: | :------------------------------------------------------------------ | :-------------------------------------------------------- |
| !0011194#   |   194    | !1Z01236#                                                           | Set led 1 to ON.                                          |
| !0041197#   |   194    | !1Z01236#                                                           | Set led 4 to ON.                                          |
| !0040196#   |   194    | !1Z01236#                                                           | Set led 4 to OFF.                                         |
| !011010291# |   291    | !1Z01236#                                                           | Set led 1 and 3 to ON, 2 and 4 to Off. (atomic operation) |
| !02098#     |   098    | !1A0101259#                                                         | Read the value of the digital inputs                      |
| !03099#     |   099    | !1B1010260#                                                         | Read the value of the digital outputs                     |
| !04100#     |   100    | !1C+25213#                                                          | Read the last value of temperature                        |
| !05101#     |   101    | !1D+25+25+25+25+25+25+25+25+25+25+25+25+25+25+25+25+25+25+25+25988# | Read the last 20 values of temperature                    |
| !06102#     |   102    | !1E+100-100447#                                                     | Read the max and min temperature                          |
| !07103#     |   103    | !1Z01236#                                                           | Reset the temperature history (including max and min)     |
| #07101!     |   103    | !1Z7452#                                                            | Invalid frame                                             |
| !0011134#   |   194    | !1Z0351#                                                            | Invalid Checksum                                          |
| !0G11134#   |   194    | !1ZG250#                                                            | Unknown Command                                           |
