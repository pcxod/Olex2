#include "../henke.h"
#define NOVAL cm_Anomalous_Henke::Undefined
static const cm_Anomalous_Henke XlibObject(_cm_henke_Xe)[] = {
  {10, NOVAL, 0},
  {10.1617, NOVAL, 0},
  {10.3261, NOVAL, 0},
  {10.4931, NOVAL, 0},
  {10.6628, NOVAL, 0},
  {10.8353, NOVAL, 0},
  {11.0106, NOVAL, 0},
  {11.1886, NOVAL, 0},
  {11.3696, NOVAL, 0},
  {11.5535, NOVAL, 0},
  {11.7404, NOVAL, 0},
  {11.9303, NOVAL, 0},
  {12, NOVAL, 0},
  {12.2, NOVAL, 0.000031},
  {12.3193, NOVAL, 1.78551},
  {12.5186, NOVAL, 1.86163},
  {12.721, NOVAL, 7.89998},
  {12.9268, NOVAL, 9.28777},
  {13.1359, NOVAL, 10.1806},
  {13.3483, NOVAL, 10.845},
  {13.5642, NOVAL, 11.4783},
  {13.7836, NOVAL, 11.9987},
  {14.0066, NOVAL, 12.5503},
  {14.2331, NOVAL, 13.1464},
  {14.4633, NOVAL, 13.3356},
  {14.6973, NOVAL, 13.4705},
  {14.935, NOVAL, 13.6068},
  {15.1765, NOVAL, 13.7445},
  {15.422, NOVAL, 13.7867},
  {15.6714, NOVAL, 13.6926},
  {15.9249, NOVAL, 13.5991},
  {16.1825, NOVAL, 13.5063},
  {16.4442, NOVAL, 13.4141},
  {16.7102, NOVAL, 13.2302},
  {16.9805, NOVAL, 12.9384},
  {17.2551, NOVAL, 12.653},
  {17.5342, NOVAL, 12.3738},
  {17.8178, NOVAL, 12.1009},
  {18.106, NOVAL, 11.834},
  {18.3989, NOVAL, 11.5597},
  {18.6964, NOVAL, 11.262},
  {18.9988, NOVAL, 10.972},
  {19.3061, NOVAL, 10.6895},
  {19.6184, NOVAL, 10.4142},
  {19.9357, NOVAL, 10.146},
  {20.2582, NOVAL, 9.88472},
  {20.5858, NOVAL, 9.63017},
  {20.9188, NOVAL, 9.38218},
  {21.2571, NOVAL, 9.14057},
  {21.6009, NOVAL, 8.90518},
  {21.9503, NOVAL, 8.65326},
  {22.3053, NOVAL, 8.24073},
  {22.6661, NOVAL, 7.84788},
  {23.0327, NOVAL, 7.47375},
  {23.4053, NOVAL, 7.11745},
  {23.7838, NOVAL, 6.77814},
  {24.1685, NOVAL, 6.455},
  {24.5594, NOVAL, 6.14727},
  {24.9566, NOVAL, 5.85421},
  {25.3603, NOVAL, 5.57512},
  {25.7705, NOVAL, 5.30934},
  {26.1873, NOVAL, 5.05623},
  {26.6109, NOVAL, 4.81518},
  {27.0413, NOVAL, 4.58563},
  {27.4786, NOVAL, 4.35879},
  {27.9231, NOVAL, 4.14242},
  {28.3747, NOVAL, 3.93679},
  {28.8337, NOVAL, 3.74136},
  {29.3, 9.24787, 3.55564},
  {29.7739, 9.15401, 3.37914},
  {30.2555, 9.05665, 3.21139},
  {30.7449, 8.95659, 3.05198},
  {31.2421, 8.85495, 2.90047},
  {31.7475, 8.75252, 2.75649},
  {32.2609, 8.66863, 2.61117},
  {32.7827, 8.56578, 2.44432},
  {33.313, 8.43921, 2.28813},
  {33.8518, 8.30286, 2.14192},
  {34.3993, 8.15845, 2.00505},
  {34.9557, 8.00686, 1.87693},
  {35.5211, 7.84855, 1.75699},
  {36.0956, 7.68358, 1.64472},
  {36.6794, 7.50428, 1.53963},
  {37.2727, 7.32127, 1.45808},
  {37.8755, 7.14335, 1.38499},
  {38.4882, 6.96445, 1.31556},
  {39.1107, 6.77222, 1.24961},
  {39.7432, 6.57704, 1.20822},
  {40.3861, 6.39261, 1.1765},
  {41.0393, 6.21168, 1.14562},
  {41.7031, 6.02982, 1.11554},
  {42.3776, 5.84214, 1.08702},
  {43.063, 5.65382, 1.06731},
  {43.7595, 5.46429, 1.04795},
  {44.4673, 5.27081, 1.02894},
  {45.1865, 5.07147, 1.01028},
  {45.9174, 4.85931, 0.991962},
  {46.66, 4.64081, 0.985351},
  {47.4147, 4.41909, 0.980027},
  {48.1816, 4.18957, 0.974732},
  {48.9609, 3.94228, 0.970576},
  {49.7528, 3.68733, 0.97968},
  {50.5576, 3.42626, 0.98887},
  {51.3753, 3.15172, 0.998145},
  {52.2062, 2.85945, 1.01157},
  {53.0506, 2.55235, 1.02998},
  {53.9087, 2.2278, 1.04873},
  {54.7806, 1.87892, 1.06781},
  {55.6667, 1.49957, 1.09067},
  {56.567, 1.0879, 1.11783},
  {57.482, 0.635603, 1.14567},
  {58.4117, 0.128885, 1.1742},
  {59.3564, -0.469527, 1.20343},
  {60.3165, -1.17074, 1.27189},
  {61.2921, -2.01898, 1.35847},
  {62.2834, -3.32953, 1.5484},
  {63.2908, -4.78492, 2.60003},
  {64.3145, -6.07324, 4.36594},
  {65.3547, -4.99111, 7.33114},
  {66.4118, -3.14174, 7.31205},
  {67.4859, -2.75421, 6.32607},
  {68.5775, -3.3389, 5.47306},
  {69.6867, -4.96733, 5.26332},
  {70.8138, -6.42927, 6.13015},
  {71.9591, -7.44364, 7.13975},
  {73.123, -8.36901, 8.3156},
  {74.3057, -9.20709, 9.68514},
  {75.5076, -9.88767, 11.2802},
  {76.7289, -10.2734, 12.9916},
  {77.9699, -10.5619, 14.7116},
  {79.231, -10.5638, 16.6594},
  {80.5125, -10.3859, 18.4108},
  {81.8147, -10.1979, 20.2492},
  {83.138, -9.86862, 22.2711},
  {84.4827, -8.89494, 24.3769},
  {85.8491, -7.87394, 25.9997},
  {87.2377, -6.97134, 27.7306},
  {88.6487, -5.91492, 29.5766},
  {90.0825, -4.32097, 31.5456},
  {91.5395, -2.54196, 32.9879},
  {93.0201, -0.817933, 34.395},
  {94.5246, 1.0918, 35.8621},
  {96.0535, 3.61712, 37.0881},
  {97.6071, 6.04274, 37.6383},
  {99.1858, 8.20609, 38.1965},
  {100.79, 10.4756, 38.7631},
  {102.42, 13.1326, 39.104},
  {104.077, 15.6864, 38.9448},
  {105.76, 18.0665, 38.7863},
  {107.471, 20.7633, 38.6284},
  {109.209, 23.4217, 37.6772},
  {110.975, 25.7865, 36.6323},
  {112.77, 28.6833, 35.6163},
  {114.594, 31.149, 33.2117},
  {116.448, 32.7183, 30.9479},
  {118.331, 33.9623, 28.8384},
  {120.245, 35.1408, 26.8726},
  {122.19, 36.4739, 24.7408},
  {124.166, 37.1409, 21.9711},
  {126.175, 37.2229, 19.5113},
  {128.215, 37.0163, 17.3269},
  {130.289, 36.618, 15.3871},
  {132.397, 36.0981, 13.6645},
  {134.538, 35.4897, 12.1347},
  {136.714, 34.827, 10.7762},
  {138.925, 34.119, 9.56974},
  {141.172, 33.3983, 8.51687},
  {143.456, 32.6755, 7.58211},
  {145.776, 31.9591, 6.74997},
  {148.134, 31.2477, 6.00916},
  {150.53, 30.5484, 5.34963},
  {152.964, 29.8693, 4.75853},
  {155.439, 29.1829, 4.20373},
  {157.953, 28.4795, 3.71361},
  {160.507, 27.7506, 3.30996},
  {163.103, 27.0554, 3.00803},
  {165.742, 26.3968, 2.73365},
  {168.422, 25.7007, 2.50814},
  {171.146, 25.0692, 2.4229},
  {173.915, 24.501, 2.34056},
  {176.727, 23.9171, 2.29085},
  {179.586, 23.3883, 2.32877},
  {182.491, 22.9277, 2.36731},
  {185.442, 22.4983, 2.42192},
  {188.442, 22.0995, 2.48194},
  {191.489, 21.7114, 2.54345},
  {194.587, 21.3291, 2.64107},
  {197.734, 20.9806, 2.7693},
  {200.932, 20.6692, 2.90375},
  {204.182, 20.3805, 3.04473},
  {207.485, 20.1117, 3.19362},
  {210.84, 19.8625, 3.35046},
  {214.251, 19.6316, 3.515},
  {217.716, 19.4184, 3.68762},
  {221.237, 19.2267, 3.86872},
  {224.816, 19.0517, 4.05053},
  {228.452, 18.8896, 4.24019},
  {232.147, 18.7442, 4.43874},
  {235.902, 18.6171, 4.64658},
  {239.717, 18.5106, 4.86416},
  {243.595, 18.446, 5.09193},
  {247.535, 18.4112, 5.29354},
  {251.538, 18.3738, 5.47387},
  {255.607, 18.3395, 5.66034},
  {259.741, 18.3173, 5.85316},
  {263.942, 18.3114, 6.05255},
  {268.211, 18.3514, 6.25872},
  {272.549, 18.4216, 6.42272},
  {276.957, 18.4756, 6.54819},
  {281.437, 18.5193, 6.67611},
  {285.989, 18.5652, 6.80654},
  {290.615, 18.6195, 6.93951},
  {295.315, 18.6897, 7.07508},
  {300.092, 18.8057, 7.21331},
  {304.945, 18.9234, 7.2606},
  {309.878, 19.008, 7.29716},
  {314.89, 19.0713, 7.33392},
  {319.983, 19.1248, 7.37085},
  {325.158, 19.1713, 7.40797},
  {330.418, 19.2124, 7.44529},
  {335.762, 19.2493, 7.48278},
  {341.192, 19.2829, 7.52047},
  {346.711, 19.3138, 7.55835},
  {352.319, 19.3426, 7.59641},
  {358.017, 19.3701, 7.63467},
  {363.808, 19.3974, 7.67313},
  {369.692, 19.4263, 7.71177},
  {375.672, 19.4655, 7.75061},
  {381.748, 19.5031, 7.76071},
  {387.922, 19.5253, 7.76418},
  {394.197, 19.5345, 7.76765},
  {400.573, 19.5356, 7.77112},
  {407.052, 19.5294, 7.7746},
  {413.635, 19.5164, 7.77807},
  {420.326, 19.4972, 7.78154},
  {427.124, 19.4722, 7.78503},
  {434.032, 19.4422, 7.7885},
  {441.052, 19.4082, 7.79198},
  {448.186, 19.3783, 7.79547},
  {455.435, 19.3645, 7.78776},
  {462.802, 19.3252, 7.7196},
  {470.287, 19.2438, 7.65206},
  {477.894, 19.1344, 7.5851},
  {485.623, 18.9992, 7.51873},
  {493.478, 18.8383, 7.45294},
  {501.459, 18.6506, 7.38772},
  {509.57, 18.4343, 7.32308},
  {517.812, 18.1867, 7.25901},
  {526.187, 17.9046, 7.19549},
  {534.698, 17.5839, 7.13252},
  {543.346, 17.2149, 7.07011},
  {552.134, 16.7959, 7.01671},
  {561.065, 16.3253, 6.96779},
  {570.139, 15.7908, 6.91921},
  {579.361, 15.1775, 6.87097},
  {588.732, 14.4668, 6.82307},
  {598.254, 13.6367, 6.7755},
  {607.93, 12.6493, 6.72163},
  {617.763, 11.4392, 6.65668},
  {627.755, 9.91544, 6.59235},
  {637.908, 7.92458, 6.52865},
  {648.226, 5.15405, 6.46555},
  {658.711, 0.823389, 6.40307},
  {669.365, -8.21883, 6.34119},
  {676.3, -53.1265, 6.30176},
  {676.5, -53.0765, 40.0443},
  {680.191, -13.5912, 39.7162},
  {691.193, 1.88287, 38.7644},
  {702.372, 8.50978, 37.8355},
  {713.733, 12.8348, 36.9287},
  {725.277, 16.0447, 36.0437},
  {737.008, 18.5805, 35.18},
  {748.928, 20.6559, 34.3369},
  {761.042, 22.3906, 33.514},
  {773.351, 23.846, 32.7108},
  {785.859, 25.0739, 31.9493},
  {798.57, 26.1402, 31.2468},
  {811.486, 27.0729, 30.5598},
  {824.611, 27.8731, 29.8878},
  {837.949, 28.5435, 29.2306},
  {851.502, 29.0794, 28.5879},
  {865.274, 29.4623, 27.9593},
  {879.269, 29.6403, 27.3445},
  {893.491, 29.3691, 26.7433},
  {907.943, 28.8453, 27.122},
  {922.628, 28.8239, 27.9663},
  {937.551, 29.3445, 28.8369},
  {952.715, 30.2131, 29.7346},
  {968.124, 31.8753, 30.4358},
  {983.783, 33.5306, 29.9975},
  {999.695, 34.5075, 29.5656},
  {1015.86, 35.3891, 29.1399},
  {1032.29, 36.1745, 28.7203},
  {1048.99, 36.8886, 28.3081},
  {1065.96, 37.5503, 27.9085},
  {1083.2, 38.1725, 27.5145},
  {1100.72, 38.7608, 27.1261},
  {1118.52, 39.3204, 26.743},
  {1136.61, 39.8555, 26.3655},
  {1155, 40.3704, 25.9932},
  {1173.68, 40.8685, 25.626},
  {1192.66, 41.3542, 25.2644},
  {1211.95, 41.8336, 24.9077},
  {1231.55, 42.3199, 24.556},
  {1251.47, 42.8247, 24.1689},
  {1271.72, 43.3132, 23.7415},
  {1292.29, 43.748, 23.2653},
  {1313.19, 44.1298, 22.7985},
  {1334.43, 44.4783, 22.3412},
  {1356.01, 44.7999, 21.8932},
  {1377.94, 45.0988, 21.454},
  {1400.23, 45.3774, 21.0235},
  {1422.88, 45.6376, 20.6017},
  {1445.89, 45.8813, 20.1883},
  {1469.28, 46.1096, 19.7834},
  {1493.04, 46.3237, 19.3868},
  {1517.19, 46.525, 18.9992},
  {1541.73, 46.715, 18.6195},
  {1566.67, 46.8945, 18.2476},
  {1592.01, 47.0641, 17.8828},
  {1617.76, 47.224, 17.5254},
  {1643.92, 47.3755, 17.1753},
  {1670.51, 47.5192, 16.8318},
  {1697.53, 47.6555, 16.4953},
  {1724.99, 47.7891, 16.1656},
  {1752.89, 47.9166, 15.8343},
  {1781.24, 48.0323, 15.507},
  {1810.05, 48.1391, 15.1843},
  {1839.32, 48.2365, 14.868},
  {1869.07, 48.3262, 14.5584},
  {1899.3, 48.4094, 14.2557},
  {1930.02, 48.4866, 13.9588},
  {1961.24, 48.5576, 13.6681},
  {1992.96, 48.6218, 13.3835},
  {2025.2, 48.6862, 13.1109},
  {2057.95, 48.7501, 12.8333},
  {2091.24, 48.8033, 12.5599},
  {2125.06, 48.8512, 12.2914},
  {2159.43, 48.8936, 12.0278},
  {2194.36, 48.9308, 11.7689},
  {2229.85, 48.9633, 11.5149},
  {2265.92, 48.9913, 11.2654},
  {2302.57, 49.0146, 11.0201},
  {2339.81, 49.0333, 10.7797},
  {2377.66, 49.048, 10.5433},
  {2416.11, 49.0581, 10.3112},
  {2455.19, 49.0642, 10.0837},
  {2494.9, 49.0663, 9.85962},
  {2535.26, 49.0639, 9.64058},
  {2576.26, 49.058, 9.42503},
  {2617.93, 49.0477, 9.21347},
  {2660.27, 49.0335, 9.0062},
  {2703.3, 49.0155, 8.80231},
  {2747.03, 48.9935, 8.60254},
  {2791.46, 48.9675, 8.40617},
  {2836.61, 48.9373, 8.21365},
  {2882.49, 48.9032, 8.02469},
  {2929.11, 48.8648, 7.83922},
  {2976.48, 48.8221, 7.65729},
  {3024.63, 48.7751, 7.47879},
  {3073.55, 48.7236, 7.30363},
  {3123.26, 48.6674, 7.13165},
  {3173.78, 48.6062, 6.96295},
  {3225.11, 48.54, 6.79748},
  {3277.27, 48.4683, 6.63504},
  {3330.28, 48.3909, 6.47567},
  {3384.15, 48.3073, 6.3193},
  {3438.88, 48.2172, 6.1659},
  {3494.5, 48.12, 6.01544},
  {3551.02, 48.0151, 5.86774},
  {3608.46, 47.9017, 5.72276},
  {3666.82, 47.7789, 5.58056},
  {3726.13, 47.6459, 5.4412},
  {3786.4, 47.5013, 5.30424},
  {3847.64, 47.3434, 5.16996},
  {3909.87, 47.1705, 5.0382},
  {3973.11, 46.9802, 4.9089},
  {4037.38, 46.7695, 4.78203},
  {4102.68, 46.5345, 4.65756},
  {4169.03, 46.27, 4.5355},
  {4236.46, 45.9693, 4.41575},
  {4304.98, 45.6222, 4.29816},
  {4374.62, 45.214, 4.18271},
  {4445.37, 44.7217, 4.06953},
  {4517.27, 44.1057, 3.95854},
  {4590.33, 43.2884, 3.84952},
  {4664.58, 42.0783, 3.7426},
  {4740.03, 39.6979, 3.63771},
  {4787.1, 24.737, 3.57457},
  {4787.3, 24.7388, 11.23},
  {4816.69, 38.5346, 11.1152},
  {4894.6, 41.4727, 10.8198},
  {4973.77, 42.3694, 10.5324},
  {5054.21, 42.2444, 10.2525},
  {5107.1, 35.5416, 10.0749},
  {5107.3, 35.5459, 13.7396},
  {5135.96, 42.2243, 13.6172},
  {5219.03, 44.2811, 13.272},
  {5303.44, 45.2083, 12.9357},
  {5389.22, 45.604, 12.6079},
  {5452.7, 42.4369, 12.3739},
  {5452.9, 42.4412, 14.2726},
  {5476.39, 45.7152, 14.1809},
  {5564.97, 47.251, 13.8442},
  {5654.98, 48.1223, 13.514},
  {5746.44, 48.7967, 13.1897},
  {5839.39, 49.3585, 12.8715},
  {5933.83, 49.8423, 12.5592},
  {6029.81, 50.2667, 12.253},
  {6127.33, 50.6436, 11.9527},
  {6226.44, 50.9812, 11.6584},
  {6327.15, 51.2854, 11.37},
  {6429.48, 51.5609, 11.0876},
  {6533.48, 51.8112, 10.8106},
  {6639.15, 52.039, 10.5397},
  {6746.54, 52.2471, 10.2746},
  {6855.65, 52.4372, 10.0151},
  {6966.54, 52.6112, 9.76125},
  {7079.22, 52.7706, 9.51281},
  {7193.72, 52.9165, 9.2699},
  {7310.07, 53.0501, 9.03234},
  {7428.31, 53.1725, 8.80037},
  {7548.45, 53.2846, 8.57347},
  {7670.54, 53.3871, 8.35171},
  {7794.61, 53.4806, 8.13507},
  {7920.68, 53.566, 7.92366},
  {8048.79, 53.6438, 7.71695},
  {8178.98, 53.7145, 7.51516},
  {8311.26, 53.7786, 7.3181},
  {8445.69, 53.8364, 7.12583},
  {8582.29, 53.8887, 6.93824},
  {8721.11, 53.9356, 6.75508},
  {8862.16, 53.9776, 6.57633},
  {9005.5, 54.0148, 6.40187},
  {9151.16, 54.0477, 6.23183},
  {9299.17, 54.0765, 6.06593},
  {9449.58, 54.1015, 5.90408},
  {9602.42, 54.1229, 5.74633},
  {9757.73, 54.1409, 5.59257},
  {9915.55, 54.1559, 5.4427},
  {10075.9, 54.1681, 5.2965},
  {10238.9, 54.1775, 5.15401},
  {10404.5, 54.1843, 5.01514},
  {10572.8, 54.1888, 4.87979},
  {10743.8, 54.191, 4.74792},
  {10917.6, 54.1911, 4.61942},
  {11094.2, 54.1892, 4.49423},
  {11273.6, 54.1856, 4.37228},
  {11455.9, 54.1802, 4.25349},
  {11641.2, 54.1732, 4.13778},
  {11829.5, 54.1648, 4.02508},
  {12020.8, 54.1549, 3.91534},
  {12215.3, 54.1438, 3.80846},
  {12412.8, 54.1315, 3.70439},
  {12613.6, 54.118, 3.60307},
  {12817.6, 54.1035, 3.50441},
  {13025, 54.088, 3.40837},
  {13235.6, 54.0716, 3.31487},
  {13449.7, 54.0543, 3.22386},
  {13667.2, 54.0363, 3.13527},
  {13888.3, 54.0175, 3.04904},
  {14112.9, 53.998, 2.96511},
  {14341.2, 53.9779, 2.88344},
  {14573.1, 53.9572, 2.80395},
  {14808.9, 53.936, 2.7266},
  {15048.4, 53.9142, 2.65133},
  {15291.8, 53.8919, 2.57808},
  {15539.1, 53.8692, 2.50682},
  {15790.4, 53.8461, 2.43748},
  {16045.8, 53.8225, 2.37001},
  {16305.4, 53.7986, 2.30437},
  {16569.1, 53.7743, 2.24052},
  {16837.1, 53.7497, 2.1784},
  {17109.4, 53.7247, 2.11796},
  {17386.1, 53.6995, 2.05918},
  {17667.4, 53.6739, 2.00199},
  {17953.1, 53.648, 1.94636},
  {18243.5, 53.6218, 1.89225},
  {18538.6, 53.5952, 1.83962},
  {18838.4, 53.5684, 1.78843},
  {19143.1, 53.5413, 1.73864},
  {19452.7, 53.5138, 1.69021},
  {19767.4, 53.4861, 1.64311},
  {20087.1, 53.4579, 1.5973},
  {20412, 53.4294, 1.55275},
  {20742.1, 53.4005, 1.50942},
  {21077.6, 53.3711, 1.46727},
  {21418.5, 53.3413, 1.42629},
  {21765, 53.311, 1.38643},
  {22117, 53.2801, 1.34767},
  {22474.7, 53.2486, 1.30998},
  {22838.2, 53.2165, 1.27332},
  {23207.6, 53.1836, 1.23767},
  {23583, 53.1499, 1.203},
  {23964.4, 53.1153, 1.16929},
  {24352, 53.0796, 1.1365},
  {24745.9, 53.0428, 1.10462},
  {25146.2, 53.0047, 1.07361},
  {25552.9, 52.9651, 1.04346},
  {25966.2, 52.9238, 1.01414},
  {26386.1, 52.8805, 0.985625},
  {26812.9, 52.835, 0.957896},
  {27246.6, 52.787, 0.930929},
  {27687.3, 52.7358, 0.904706},
  {28135.1, 52.6812, 0.879202},
  {28590.2, 52.6223, 0.854402},
  {29052.6, 52.5584, 0.830283},
  {29522.5, 52.4904, 0.806827},
  {30000, 52.4028, 0.784016}
};

