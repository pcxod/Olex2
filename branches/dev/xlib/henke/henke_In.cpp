#include "../henke.h"
#define NOVAL cm_Anomalous_Henke::Undefined
static const cm_Anomalous_Henke XlibObject(_cm_henke_In)[] = {
  {10, NOVAL, 2.16244},
  {10.1617, NOVAL, 2.07002},
  {10.3261, NOVAL, 1.98155},
  {10.4931, NOVAL, 1.89686},
  {10.6628, NOVAL, 1.81579},
  {10.8353, NOVAL, 1.72844},
  {11.0106, NOVAL, 1.54985},
  {11.1886, NOVAL, 1.35731},
  {11.3696, NOVAL, 0.993252},
  {11.5535, NOVAL, 0.742018},
  {11.7404, NOVAL, 0.65818},
  {11.9303, NOVAL, 0.612667},
  {12.1232, NOVAL, 0.573696},
  {12.3193, NOVAL, 0.537374},
  {12.5186, NOVAL, 0.509773},
  {12.721, NOVAL, 0.491373},
  {12.9268, NOVAL, 0.473638},
  {13.1359, NOVAL, 0.456589},
  {13.3483, NOVAL, 0.44037},
  {13.5642, NOVAL, 0.424728},
  {13.7836, NOVAL, 0.414034},
  {14.0066, NOVAL, 0.404843},
  {14.2331, NOVAL, 0.396113},
  {14.4633, NOVAL, 0.388198},
  {14.6973, NOVAL, 0.38598},
  {14.935, NOVAL, 0.387391},
  {15.1765, NOVAL, 0.396152},
  {15.422, NOVAL, 0.410419},
  {15.6714, NOVAL, 0.431951},
  {15.9249, NOVAL, 0.455912},
  {16.1825, NOVAL, 0.489651},
  {16.4442, NOVAL, 0.691634},
  {16.7102, NOVAL, 1.08283},
  {16.9805, NOVAL, 1.49226},
  {17.2551, NOVAL, 1.78423},
  {17.5342, NOVAL, 1.92144},
  {17.8178, NOVAL, 1.94167},
  {18.106, NOVAL, 1.95766},
  {18.3989, NOVAL, 1.966},
  {18.6964, NOVAL, 1.95605},
  {18.9988, NOVAL, 1.92465},
  {19.3061, NOVAL, 1.90352},
  {19.6184, NOVAL, 1.88557},
  {19.9357, NOVAL, 1.82853},
  {20.2582, NOVAL, 1.76749},
  {20.5858, NOVAL, 1.75409},
  {20.9188, NOVAL, 1.74106},
  {21.2571, NOVAL, 1.72812},
  {21.6009, NOVAL, 1.73515},
  {21.9503, NOVAL, 1.77156},
  {22.3053, NOVAL, 1.80874},
  {22.6661, NOVAL, 1.86471},
  {23.0327, NOVAL, 1.95371},
  {23.4053, NOVAL, 2.04696},
  {23.7838, NOVAL, 2.14466},
  {24.1685, NOVAL, 2.25579},
  {24.5594, NOVAL, 2.37578},
  {24.9566, NOVAL, 2.50216},
  {25.3603, NOVAL, 2.63526},
  {25.7705, NOVAL, 2.76576},
  {26.1873, NOVAL, 2.93943},
  {26.6109, NOVAL, 3.13348},
  {27.0413, NOVAL, 3.34034},
  {27.4786, NOVAL, 3.50223},
  {27.9231, NOVAL, 3.66764},
  {28.3747, NOVAL, 3.84087},
  {28.8337, NOVAL, 4.06669},
  {29.3, -1.51987, 4.32792},
  {29.7739, -1.54706, 4.56695},
  {30.2555, -1.58334, 4.75454},
  {30.7449, -1.64752, 4.94983},
  {31.2421, -1.74136, 5.16036},
  {31.7475, -1.83332, 5.42475},
  {32.2609, -1.89419, 5.70269},
  {32.7827, -1.93995, 5.99488},
  {33.313, -1.96255, 6.30145},
  {33.8518, -1.94319, 6.62114},
  {34.3993, -1.90687, 6.87184},
  {34.9557, -1.8974, 7.13204},
  {35.5211, -1.89168, 7.40209},
  {36.0956, -1.88415, 7.68236},
  {36.6794, -1.87242, 7.97325},
  {37.2727, -1.85346, 8.27515},
  {37.8755, -1.82528, 8.58848},
  {38.4882, -1.78472, 8.91368},
  {39.1107, -1.71506, 9.25118},
  {39.7432, -1.62506, 9.57113},
  {40.3861, -1.54115, 9.8802},
  {41.0393, -1.45722, 10.1993},
  {41.7031, -1.36657, 10.5286},
  {42.3776, -1.26484, 10.8686},
  {43.063, -1.14874, 11.2196},
  {43.7595, -1.01435, 11.5819},
  {44.4673, -0.82735, 11.9559},
  {45.1865, -0.617621, 12.2735},
  {45.9174, -0.436164, 12.5863},
  {46.66, -0.250289, 12.9071},
  {47.4147, -0.055007, 13.2361},
  {48.1816, 0.157037, 13.5734},
  {48.9609, 0.408463, 13.9094},
  {49.7528, 0.668244, 14.1975},
  {50.5576, 0.903882, 14.4915},
  {51.3753, 1.14556, 14.7916},
  {52.2062, 1.39512, 15.098},
  {53.0506, 1.65814, 15.4106},
  {53.9087, 1.93838, 15.7207},
  {54.7806, 2.22861, 16.0272},
  {55.6667, 2.52847, 16.3396},
  {56.567, 2.84821, 16.6582},
  {57.482, 3.19594, 16.9829},
  {58.4117, 3.59613, 17.314},
  {59.3564, 4.07115, 17.5669},
  {60.3165, 4.51316, 17.7093},
  {61.2921, 4.87293, 17.8528},
  {62.2834, 5.20917, 17.9975},
  {63.2908, 5.47784, 18.1434},
  {64.3145, 5.71312, 18.3801},
  {65.3547, 6.01602, 18.7146},
  {66.4118, 6.39303, 19.0551},
  {67.4859, 6.96027, 19.4018},
  {68.5775, 7.55437, 19.4426},
  {69.6867, 7.89693, 19.4471},
  {70.8138, 8.099, 19.4661},
  {71.9591, 8.25154, 19.781},
  {73.123, 8.51743, 20.1009},
  {74.3057, 8.6878, 20.4261},
  {75.5076, 8.85727, 21.0189},
  {76.7289, 9.27382, 21.7901},
  {77.9699, 9.89802, 22.5897},
  {79.231, 10.8442, 23.4186},
  {80.5125, 12.0436, 24.0119},
  {81.8147, 13.2334, 24.3235},
  {83.138, 14.4128, 24.6392},
  {84.4827, 15.9636, 24.9589},
  {85.8491, 17.6504, 24.661},
  {87.2377, 18.9234, 24.09},
  {88.6487, 20.0231, 23.5323},
  {90.0825, 21.04, 22.9874},
  {91.5395, 22.0277, 22.4552},
  {93.0201, 23.0747, 21.9352},
  {94.5246, 24.3349, 21.4124},
  {96.0535, 25.5304, 20.0346},
  {97.6071, 26.2006, 18.7454},
  {99.1858, 26.6544, 17.5392},
  {100.79, 26.9621, 16.4105},
  {102.42, 27.163, 15.3546},
  {104.077, 27.2812, 14.3665},
  {105.76, 27.3359, 13.4421},
  {107.471, 27.3408, 12.5771},
  {109.209, 27.3287, 11.7675},
  {110.975, 27.2772, 10.9499},
  {112.77, 27.1511, 10.1891},
  {114.594, 26.9928, 9.48115},
  {116.448, 26.8092, 8.82241},
  {118.331, 26.6063, 8.20945},
  {120.245, 26.3905, 7.63905},
  {122.19, 26.1664, 7.1083},
  {124.166, 25.9399, 6.61442},
  {126.175, 25.7178, 6.15485},
  {128.215, 25.5129, 5.72721},
  {130.289, 25.3659, 5.32929},
  {132.397, 25.2858, 4.7905},
  {134.538, 25.0639, 4.1082},
  {136.714, 24.6395, 3.5231},
  {138.925, 24.15, 3.02132},
  {141.172, 23.603, 2.63553},
  {143.456, 23.0693, 2.37563},
  {145.776, 22.578, 2.15004},
  {148.134, 22.0825, 1.97681},
  {150.53, 21.593, 1.86588},
  {152.964, 21.129, 1.81419},
  {155.439, 20.7009, 1.81256},
  {157.953, 20.3192, 1.84702},
  {160.507, 19.9783, 1.888},
  {163.103, 19.664, 1.94576},
  {165.742, 19.3783, 2.00529},
  {168.422, 19.1071, 2.06792},
  {171.146, 18.8516, 2.14723},
  {173.915, 18.6169, 2.22959},
  {176.727, 18.3966, 2.31511},
  {179.586, 18.1872, 2.40391},
  {182.491, 17.9882, 2.50249},
  {185.442, 17.8035, 2.60565},
  {188.442, 17.6302, 2.71306},
  {191.489, 17.4678, 2.8249},
  {194.587, 17.3171, 2.94135},
  {197.734, 17.1771, 3.05935},
  {200.932, 17.0445, 3.18022},
  {204.182, 16.9205, 3.30587},
  {207.485, 16.8059, 3.43648},
  {210.84, 16.7019, 3.57226},
  {214.251, 16.6106, 3.7134},
  {217.716, 16.5397, 3.86012},
  {221.237, 16.4849, 3.98186},
  {224.816, 16.4229, 4.10237},
  {228.452, 16.3626, 4.22653},
  {232.147, 16.3079, 4.35445},
  {235.902, 16.2606, 4.48624},
  {239.717, 16.2224, 4.62201},
  {243.595, 16.2096, 4.7619},
  {247.535, 16.2092, 4.872},
  {251.538, 16.1923, 4.9795},
  {255.607, 16.1785, 5.08938},
  {259.741, 16.1693, 5.20168},
  {263.942, 16.1677, 5.31645},
  {268.211, 16.1855, 5.43376},
  {272.549, 16.2257, 5.53876},
  {276.957, 16.2643, 5.59146},
  {281.437, 16.2791, 5.64465},
  {285.989, 16.2866, 5.69836},
  {290.615, 16.2914, 5.75257},
  {295.315, 16.2977, 5.80731},
  {300.092, 16.3144, 5.86256},
  {304.945, 16.3509, 5.88879},
  {309.878, 16.3676, 5.8707},
  {314.89, 16.3456, 5.85266},
  {319.983, 16.3086, 5.83467},
  {325.158, 16.258, 5.81675},
  {330.418, 16.1986, 5.79887},
  {335.762, 16.129, 5.76451},
  {341.192, 16.0338, 5.7234},
  {346.711, 15.9148, 5.68258},
  {352.319, 15.7748, 5.64204},
  {358.017, 15.6134, 5.6018},
  {363.808, 15.4305, 5.56184},
  {369.692, 15.2236, 5.51516},
  {375.672, 14.9827, 5.46471},
  {381.748, 14.7037, 5.41473},
  {387.922, 14.3835, 5.3652},
  {394.197, 14.0088, 5.31613},
  {400.573, 13.5727, 5.2797},
  {407.052, 13.0832, 5.27535},
  {413.635, 12.5374, 5.27101},
  {420.326, 11.8846, 5.26666},
  {427.124, 11.1169, 5.31364},
  {434.032, 10.2736, 5.41613},
  {441.052, 9.47596, 5.52059},
  {443.8, 10.6734, 5.56157},
  {444, 10.5639, 3.58563},
  {448.186, 6.68163, 4.49251},
  {455.435, 3.96074, 6.60583},
  {462.802, 2.52924, 9.71329},
  {470.287, 2.21022, 12.5187},
  {477.894, 1.97589, 15.3206},
  {485.623, 2.71371, 18.7498},
  {493.478, 4.8266, 20.8571},
  {501.459, 6.67054, 22.3283},
  {509.57, 8.3, 23.2112},
  {517.812, 9.80035, 24.1291},
  {526.187, 11.4742, 24.7386},
  {534.698, 12.97, 24.9583},
  {543.346, 14.3294, 25.18},
  {552.134, 15.6461, 25.0704},
  {561.065, 16.6175, 24.8845},
  {570.139, 17.4461, 24.6999},
  {579.361, 18.123, 24.5167},
  {588.732, 18.6809, 24.3966},
  {598.254, 19.2051, 24.3595},
  {607.93, 19.7052, 24.3223},
  {617.763, 20.2164, 24.2853},
  {627.755, 20.6751, 24.1094},
  {637.908, 20.6129, 23.7562},
  {648.226, 20.1002, 23.9302},
  {658.711, 19.8932, 25.1601},
  {669.365, 20.3338, 26.4532},
  {680.191, 21.4247, 27.8128},
  {691.193, 23.184, 28.8952},
  {702.372, 25.0064, 28.6033},
  {713.733, 26.2436, 28.3143},
  {725.277, 27.365, 28.0282},
  {737.008, 28.385, 27.5828},
  {748.928, 29.1658, 27.0982},
  {761.042, 29.7932, 26.6222},
  {773.351, 30.1958, 26.2179},
  {785.859, 30.5723, 26.1563},
  {798.57, 31.1509, 26.0949},
  {811.486, 31.7727, 26.0337},
  {824.611, 32.5186, 25.9725},
  {837.949, 33.2978, 25.6803},
  {851.502, 33.9336, 25.3864},
  {865.274, 34.5536, 25.0957},
  {879.269, 35.2118, 24.8084},
  {893.491, 35.8707, 24.4025},
  {907.943, 36.4108, 23.9705},
  {922.628, 36.9053, 23.546},
  {937.551, 37.3634, 23.1322},
  {952.715, 37.7949, 22.7266},
  {968.124, 38.2053, 22.3289},
  {983.783, 38.6003, 21.9382},
  {999.695, 38.9878, 21.5485},
  {1015.86, 39.3699, 21.1599},
  {1032.29, 39.7347, 20.7214},
  {1048.99, 40.0516, 20.2914},
  {1065.96, 40.341, 19.8702},
  {1083.2, 40.6079, 19.4578},
  {1100.72, 40.8556, 19.0539},
  {1118.52, 41.086, 18.6584},
  {1136.61, 41.3008, 18.2712},
  {1155, 41.5014, 17.8919},
  {1173.68, 41.6887, 17.5204},
  {1192.66, 41.8637, 17.1569},
  {1211.95, 42.0276, 16.8008},
  {1231.55, 42.181, 16.4521},
  {1251.47, 42.3243, 16.1105},
  {1271.72, 42.4582, 15.776},
  {1292.29, 42.5833, 15.4486},
  {1313.19, 42.7002, 15.1281},
  {1334.43, 42.8094, 14.8142},
  {1356.01, 42.9113, 14.5067},
  {1377.94, 43.0062, 14.2056},
  {1400.23, 43.0945, 13.9107},
  {1422.88, 43.1765, 13.6219},
  {1445.89, 43.2525, 13.3392},
  {1469.28, 43.3231, 13.0623},
  {1493.04, 43.3883, 12.7911},
  {1517.19, 43.4485, 12.5254},
  {1541.73, 43.5038, 12.265},
  {1566.67, 43.5544, 12.01},
  {1592.01, 43.6006, 11.7604},
  {1617.76, 43.6427, 11.5159},
  {1643.92, 43.6811, 11.2766},
  {1670.51, 43.716, 11.0421},
  {1697.53, 43.7477, 10.8125},
  {1724.99, 43.7788, 10.5876},
  {1752.89, 43.8079, 10.3619},
  {1781.24, 43.8295, 10.1389},
  {1810.05, 43.846, 9.91898},
  {1839.32, 43.857, 9.70402},
  {1869.07, 43.8638, 9.49377},
  {1899.3, 43.8668, 9.28813},
  {1930.02, 43.8662, 9.08683},
  {1961.24, 43.862, 8.8899},
  {1992.96, 43.8541, 8.69729},
  {2025.2, 43.8453, 8.51216},
  {2057.95, 43.8364, 8.32445},
  {2091.24, 43.8207, 8.13978},
  {2125.06, 43.801, 7.95855},
  {2159.43, 43.7774, 7.7805},
  {2194.36, 43.7497, 7.60617},
  {2229.85, 43.7185, 7.43484},
  {2265.92, 43.6831, 7.26657},
  {2302.57, 43.6436, 7.10168},
  {2339.81, 43.6001, 6.93958},
  {2377.66, 43.5521, 6.78093},
  {2416.11, 43.4998, 6.62507},
  {2455.19, 43.4429, 6.47223},
  {2494.9, 43.3812, 6.32231},
  {2535.26, 43.3143, 6.17519},
  {2576.26, 43.242, 6.0309},
  {2617.93, 43.1641, 5.88943},
  {2660.27, 43.08, 5.75061},
  {2703.3, 42.9892, 5.61447},
  {2747.03, 42.8913, 5.481},
  {2791.46, 42.7855, 5.35007},
  {2836.61, 42.671, 5.2217},
  {2882.49, 42.5469, 5.09578},
  {2929.11, 42.4121, 4.97234},
  {2976.48, 42.2651, 4.85124},
  {3024.63, 42.1043, 4.73253},
  {3073.55, 41.9276, 4.61615},
  {3123.26, 41.7323, 4.50194},
  {3173.78, 41.515, 4.39002},
  {3225.11, 41.2712, 4.28029},
  {3277.27, 40.9951, 4.1727},
  {3330.28, 40.6781, 4.06717},
  {3384.15, 40.3084, 3.96369},
  {3438.88, 39.8676, 3.8623},
  {3494.5, 39.3256, 3.76282},
  {3551.02, 38.6274, 3.66533},
  {3608.46, 37.6541, 3.56968},
  {3666.82, 36.0539, 3.47592},
  {3726.13, 29.5173, 3.38402},
  {3730, 21.0532, 3.37816},
  {3730.2, 21.055, 10.6791},
  {3786.4, 35.5004, 10.4378},
  {3847.64, 36.729, 10.185},
  {3909.87, 36.4537, 9.93807},
  {3937.9, 30.3105, 9.83012},
  {3938.1, 30.316, 13.5004},
  {3973.11, 37.5132, 13.3113},
  {4037.38, 39.2653, 12.9752},
  {4102.68, 40.1923, 12.6475},
  {4169.03, 40.7033, 12.3288},
  {4236.46, 39.079, 12.0185},
  {4237.4, 37.9036, 12.0143},
  {4237.6, 37.9089, 13.8395},
  {4304.98, 41.9828, 13.5217},
  {4374.62, 42.9091, 13.2059},
  {4445.37, 43.6018, 12.8955},
  {4517.27, 44.1731, 12.5906},
  {4590.33, 44.6635, 12.291},
  {4664.58, 45.0929, 11.9967},
  {4740.03, 45.4743, 11.7078},
  {4816.69, 45.8161, 11.4243},
  {4894.6, 46.1245, 11.1464},
  {4973.77, 46.404, 10.8737},
  {5054.21, 46.6582, 10.6064},
  {5135.96, 46.8902, 10.3445},
  {5219.03, 47.1024, 10.0879},
  {5303.44, 47.2967, 9.83652},
  {5389.22, 47.4747, 9.59033},
  {5476.39, 47.638, 9.34942},
  {5564.97, 47.7881, 9.11368},
  {5654.98, 47.9259, 8.88285},
  {5746.44, 48.0522, 8.65718},
  {5839.39, 48.1683, 8.43646},
  {5933.83, 48.2747, 8.22062},
  {6029.81, 48.3723, 8.00956},
  {6127.33, 48.4615, 7.8033},
  {6226.44, 48.5431, 7.6018},
  {6327.15, 48.6177, 7.40489},
  {6429.48, 48.6855, 7.21256},
  {6533.48, 48.7471, 7.0247},
  {6639.15, 48.803, 6.8412},
  {6746.54, 48.8535, 6.66207},
  {6855.65, 48.8989, 6.48721},
  {6966.54, 48.9397, 6.31649},
  {7079.22, 48.9759, 6.14986},
  {7193.72, 49.008, 5.98724},
  {7310.07, 49.0362, 5.8287},
  {7428.31, 49.0609, 5.67408},
  {7548.45, 49.0822, 5.52315},
  {7670.54, 49.1003, 5.37591},
  {7794.61, 49.1154, 5.23241},
  {7920.68, 49.1277, 5.09244},
  {8048.79, 49.1374, 4.95604},
  {8178.98, 49.1446, 4.82305},
  {8311.26, 49.1497, 4.69338},
  {8445.69, 49.1525, 4.56687},
  {8582.29, 49.1532, 4.44387},
  {8721.11, 49.1522, 4.32375},
  {8862.16, 49.1493, 4.20679},
  {9005.5, 49.1447, 4.09286},
  {9151.16, 49.1386, 3.98194},
  {9299.17, 49.1312, 3.8739},
  {9449.58, 49.1224, 3.76859},
  {9602.42, 49.1124, 3.66588},
  {9757.73, 49.1009, 3.56601},
  {9915.55, 49.0885, 3.46895},
  {10075.9, 49.0753, 3.37423},
  {10238.9, 49.061, 3.28199},
  {10404.5, 49.0458, 3.19219},
  {10572.8, 49.0298, 3.10476},
  {10743.8, 49.013, 3.01965},
  {10917.6, 48.9955, 2.9368},
  {11094.2, 48.9773, 2.85615},
  {11273.6, 48.9585, 2.77765},
  {11455.9, 48.9391, 2.70125},
  {11641.2, 48.9191, 2.62689},
  {11829.5, 48.8987, 2.55452},
  {12020.8, 48.8777, 2.4841},
  {12215.3, 48.8563, 2.41557},
  {12412.8, 48.8345, 2.34889},
  {12613.6, 48.8123, 2.284},
  {12817.6, 48.7898, 2.22087},
  {13025, 48.7668, 2.15945},
  {13235.6, 48.7436, 2.09969},
  {13449.7, 48.72, 2.04155},
  {13667.2, 48.6961, 1.98498},
  {13888.3, 48.6719, 1.92996},
  {14112.9, 48.6474, 1.87643},
  {14341.2, 48.6226, 1.82436},
  {14573.1, 48.5976, 1.77371},
  {14808.9, 48.5722, 1.72444},
  {15048.4, 48.5465, 1.67652},
  {15291.8, 48.5206, 1.6299},
  {15539.1, 48.4943, 1.58457},
  {15790.4, 48.4677, 1.54047},
  {16045.8, 48.4408, 1.49758},
  {16305.4, 48.4135, 1.45587},
  {16569.1, 48.3858, 1.4153},
  {16837.1, 48.3577, 1.37585},
  {17109.4, 48.3292, 1.33747},
  {17386.1, 48.3002, 1.30016},
  {17667.4, 48.2706, 1.26386},
  {17953.1, 48.2405, 1.22857},
  {18243.5, 48.2097, 1.19424},
  {18538.6, 48.1782, 1.16086},
  {18838.4, 48.1459, 1.1284},
  {19143.1, 48.1128, 1.09683},
  {19452.7, 48.0786, 1.06613},
  {19767.4, 48.0434, 1.03628},
  {20087.1, 48.0069, 1.00725},
  {20412, 47.9691, 0.979015},
  {20742.1, 47.9297, 0.95156},
  {21077.6, 47.8884, 0.92486},
  {21418.5, 47.8451, 0.898896},
  {21765, 47.7994, 0.873646},
  {22117, 47.7509, 0.849092},
  {22474.7, 47.6992, 0.825214},
  {22838.2, 47.6437, 0.801993},
  {23207.6, 47.5835, 0.779411},
  {23583, 47.5179, 0.757449},
  {23964.4, 47.4455, 0.736093},
  {24352, 47.3646, 0.715322},
  {24745.9, 47.273, 0.695124},
  {25146.2, 47.1672, 0.675479},
  {25552.9, 47.0422, 0.656375},
  {25966.2, 46.8897, 0.637795},
  {26386.1, 46.6953, 0.619724},
  {26812.9, 46.4295, 0.60215},
  {27246.6, 46.0168, 0.585057},
  {27687.3, 45.1261, 0.568432},
  {27939.8, 37.8125, 0.559235},
  {27940, 37.8127, 3.51485},
  {28135.1, 44.9797, 3.47697},
  {28590.2, 46.1704, 3.39119},
  {29052.6, 46.7166, 3.30702},
  {29522.5, 47.0865, 3.22447},
  {30000, 47.3215, 3.14355}
};

