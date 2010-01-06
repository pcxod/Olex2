#include "../henke.h"
#define NOVAL cm_Anomalous_Henke::Undefined
static const cm_Anomalous_Henke XlibObject(_cm_henke_Po)[] = {
  {10, NOVAL, 4.92763},
  {10.1617, NOVAL, 4.95784},
  {10.3261, NOVAL, 4.98823},
  {10.4931, NOVAL, 5.01881},
  {10.6628, NOVAL, 5.04957},
  {10.8353, NOVAL, 5.10634},
  {11.0106, NOVAL, 5.17368},
  {11.1886, NOVAL, 5.24191},
  {11.3696, NOVAL, 5.31104},
  {11.5535, NOVAL, 5.38108},
  {11.7404, NOVAL, 5.41889},
  {11.9303, NOVAL, 5.44883},
  {12.1232, NOVAL, 5.47894},
  {12.3193, NOVAL, 5.50922},
  {12.5186, NOVAL, 5.53966},
  {12.721, NOVAL, 5.52245},
  {12.9268, NOVAL, 5.48902},
  {13.1359, NOVAL, 5.4558},
  {13.3483, NOVAL, 5.42278},
  {13.5642, NOVAL, 5.38984},
  {13.7836, NOVAL, 5.35112},
  {14.0066, NOVAL, 5.31267},
  {14.2331, NOVAL, 5.27449},
  {14.4633, NOVAL, 5.2366},
  {14.6973, NOVAL, 5.19897},
  {14.935, NOVAL, 5.17213},
  {15.1765, NOVAL, 5.14602},
  {15.422, NOVAL, 5.12004},
  {15.6714, NOVAL, 5.09419},
  {15.9249, NOVAL, 5.06848},
  {16.1825, NOVAL, 5.04289},
  {16.4442, NOVAL, 5.01793},
  {16.7102, NOVAL, 4.99641},
  {16.9805, NOVAL, 4.97498},
  {17.2551, NOVAL, 4.95363},
  {17.5342, NOVAL, 4.93239},
  {17.8178, NOVAL, 4.84446},
  {18.106, NOVAL, 4.71919},
  {18.3989, NOVAL, 4.59717},
  {18.6964, NOVAL, 4.47829},
  {18.9988, NOVAL, 4.36091},
  {19.3061, NOVAL, 4.24339},
  {19.6184, NOVAL, 4.12903},
  {19.9357, NOVAL, 4.01776},
  {20.2582, NOVAL, 3.90948},
  {20.5858, NOVAL, 3.78132},
  {20.9188, NOVAL, 3.65318},
  {21.2571, NOVAL, 3.52938},
  {21.6009, NOVAL, 3.39749},
  {21.9503, NOVAL, 3.18359},
  {22.3053, NOVAL, 2.98315},
  {22.6661, NOVAL, 2.79534},
  {23.0327, NOVAL, 2.638},
  {23.4053, NOVAL, 2.50103},
  {23.7838, NOVAL, 2.37117},
  {24.1685, NOVAL, 2.24361},
  {24.5594, NOVAL, 2.11749},
  {24.9566, NOVAL, 1.99846},
  {25.3603, NOVAL, 1.89902},
  {25.7705, NOVAL, 1.81138},
  {26.1873, NOVAL, 1.72779},
  {26.6109, NOVAL, 1.71417},
  {27.0413, NOVAL, 1.71027},
  {27.4786, NOVAL, 1.69468},
  {27.9231, NOVAL, 1.66248},
  {28.3747, NOVAL, 1.65659},
  {28.8337, NOVAL, 1.69668},
  {29.3, -0.327474, 1.73976},
  {29.7739, -0.805751, 1.80621},
  {30.2555, -1.43387, 1.8752},
  {30.7449, -2.38848, 2.31378},
  {31.2421, -3.058, 3.36804},
  {31.7475, -2.95232, 4.85098},
  {32.2609, -2.42557, 5.18313},
  {32.7827, -2.15657, 5.50492},
  {33.313, -1.99161, 5.81193},
  {33.8518, -1.77401, 6.13534},
  {34.3993, -1.56068, 6.24756},
  {34.9557, -1.42888, 6.36182},
  {35.5211, -1.31019, 6.44993},
  {36.0956, -1.2607, 6.48306},
  {36.6794, -1.27061, 6.51636},
  {37.2727, -1.32712, 6.54968},
  {37.8755, -1.43392, 6.58315},
  {38.4882, -1.61752, 6.6168},
  {39.1107, -1.85359, 6.76708},
  {39.7432, -2.06699, 6.93913},
  {40.3861, -2.29738, 7.11555},
  {41.0393, -2.59566, 7.34485},
  {41.7031, -2.86185, 7.64536},
  {42.3776, -3.13525, 7.95816},
  {43.063, -3.46763, 8.28449},
  {43.7595, -3.79179, 8.80056},
  {44.4673, -4.04438, 9.34878},
  {45.1865, -4.25659, 9.93114},
  {45.9174, -4.43704, 10.5573},
  {46.66, -4.56956, 11.2294},
  {47.4147, -4.64949, 11.9442},
  {48.1816, -4.65522, 12.7045},
  {48.9609, -4.52965, 13.5132},
  {49.7528, -4.34402, 14.1494},
  {50.5576, -4.19653, 14.8},
  {51.3753, -4.0375, 15.4805},
  {52.2062, -3.84489, 16.1923},
  {53.0506, -3.59161, 16.9368},
  {53.9087, -3.22109, 17.7156},
  {54.7806, -2.81138, 18.3309},
  {55.6667, -2.42303, 18.9672},
  {56.567, -2.00624, 19.6255},
  {57.482, -1.52233, 20.3068},
  {58.4117, -0.879052, 20.9032},
  {59.3564, -0.325593, 21.37},
  {60.3165, 0.19699, 21.8473},
  {61.2921, 0.718284, 22.3352},
  {62.2834, 1.25234, 22.834},
  {63.2908, 1.80921, 23.3439},
  {64.3145, 2.39899, 23.8653},
  {65.3547, 3.03383, 24.3982},
  {66.4118, 3.73253, 24.9431},
  {67.4859, 4.53599, 25.5002},
  {68.5775, 5.58237, 25.9171},
  {69.6867, 6.47825, 26.0704},
  {70.8138, 7.28794, 26.2245},
  {71.9591, 8.06114, 26.3795},
  {73.123, 8.82059, 26.5355},
  {74.3057, 9.5783, 26.6924},
  {75.5076, 10.3441, 26.8501},
  {76.7289, 11.1279, 27.0089},
  {77.9699, 11.9438, 27.1686},
  {79.231, 12.8232, 27.3292},
  {80.5125, 13.8009, 27.3829},
  {81.8147, 14.7083, 27.3444},
  {83.138, 15.5997, 27.3059},
  {84.4827, 16.5087, 27.2675},
  {85.8491, 17.4657, 27.2291},
  {87.2377, 18.5275, 27.1908},
  {88.6487, 19.8967, 27.0619},
  {90.0825, 21.0884, 26.2733},
  {91.5395, 21.9944, 25.5076},
  {93.0201, 22.6838, 24.7826},
  {94.5246, 23.3378, 24.1931},
  {96.0535, 24.0121, 23.6176},
  {97.6071, 24.7595, 23.0557},
  {99.1858, 25.6429, 22.1924},
  {100.79, 26.1814, 21.1428},
  {102.42, 26.4173, 20.143},
  {104.077, 26.5325, 19.4083},
  {105.76, 26.7127, 18.7638},
  {107.471, 26.9059, 18.1408},
  {109.209, 27.1002, 17.5384},
  {110.975, 27.3057, 16.9561},
  {112.77, 27.5824, 16.393},
  {114.594, 27.8933, 15.6178},
  {116.448, 27.9881, 14.7636},
  {118.331, 27.9509, 13.9562},
  {120.245, 27.8092, 13.1928},
  {122.19, 27.5257, 12.4713},
  {124.166, 27.1079, 12.0324},
  {126.175, 26.8467, 11.7436},
  {128.215, 26.69, 11.4618},
  {130.289, 26.5711, 11.1867},
  {132.397, 26.4807, 10.9182},
  {134.538, 26.4151, 10.6562},
  {136.714, 26.3753, 10.4005},
  {138.925, 26.3711, 10.1509},
  {141.172, 26.4587, 9.90726},
  {143.456, 26.6053, 9.46653},
  {145.776, 26.5873, 8.93611},
  {148.134, 26.4798, 8.4354},
  {150.53, 26.318, 7.96273},
  {152.964, 26.115, 7.51656},
  {155.439, 25.8761, 7.09539},
  {157.953, 25.5938, 6.70403},
  {160.507, 25.3046, 6.37063},
  {163.103, 25.011, 6.0538},
  {165.742, 24.7073, 5.75274},
  {168.422, 24.3887, 5.46663},
  {171.146, 24.0517, 5.19477},
  {173.915, 23.6892, 4.93642},
  {176.727, 23.256, 4.69093},
  {179.586, 22.8275, 4.58996},
  {182.491, 22.4577, 4.50514},
  {185.442, 22.1118, 4.4219},
  {188.442, 21.7855, 4.33665},
  {191.489, 21.4508, 4.23282},
  {194.587, 21.0987, 4.13148},
  {197.734, 20.7266, 4.03256},
  {200.932, 20.3288, 3.9491},
  {204.182, 19.9121, 3.87586},
  {207.485, 19.4357, 3.83258},
  {210.84, 18.9677, 3.84129},
  {214.251, 18.4789, 3.881},
  {217.716, 18.0052, 3.95929},
  {221.237, 17.525, 4.03917},
  {224.816, 16.9986, 4.15828},
  {228.452, 16.4894, 4.33461},
  {232.147, 15.9546, 4.51842},
  {235.902, 15.4208, 4.81145},
  {239.717, 14.9168, 5.13299},
  {243.595, 14.4371, 5.48218},
  {247.535, 13.9687, 5.85511},
  {251.538, 13.4978, 6.25343},
  {255.607, 13.0316, 6.70406},
  {259.741, 12.5866, 7.19681},
  {263.942, 12.1604, 7.72577},
  {268.211, 11.7503, 8.29361},
  {272.549, 11.3548, 8.90874},
  {276.957, 10.9943, 9.58276},
  {281.437, 10.6797, 10.3078},
  {285.989, 10.4559, 11.0877},
  {290.615, 10.2855, 11.8188},
  {295.315, 10.1199, 12.5736},
  {300.092, 9.9933, 13.3766},
  {304.945, 9.98481, 14.2309},
  {309.878, 10.0378, 14.9726},
  {314.89, 10.0615, 15.7051},
  {319.983, 10.0997, 16.4735},
  {325.158, 10.1972, 17.2794},
  {330.418, 10.4086, 18.049},
  {335.762, 10.5784, 18.7253},
  {341.192, 10.7303, 19.427},
  {346.711, 10.8932, 20.155},
  {352.319, 11.0837, 20.9103},
  {358.017, 11.3585, 21.6939},
  {363.808, 11.6586, 22.3856},
  {369.692, 11.9331, 23.0797},
  {375.672, 12.2118, 23.7953},
  {381.748, 12.5081, 24.5331},
  {387.922, 12.8319, 25.2937},
  {394.197, 13.1919, 26.078},
  {400.573, 13.5993, 26.8865},
  {407.052, 14.1289, 27.7121},
  {413.635, 14.6723, 28.4175},
  {420.326, 15.1989, 29.1408},
  {427.124, 15.7502, 29.8825},
  {434.032, 16.3413, 30.6431},
  {441.052, 16.9892, 31.4231},
  {448.186, 17.7201, 32.2229},
  {455.435, 18.6394, 32.9905},
  {462.802, 19.5263, 33.5574},
  {470.287, 20.3925, 34.134},
  {477.894, 21.2885, 34.7205},
  {485.623, 22.2494, 35.3171},
  {493.478, 23.4623, 35.8233},
  {501.459, 24.5879, 36.0698},
  {509.57, 25.6047, 36.3181},
  {517.812, 26.6263, 36.5681},
  {526.187, 27.8029, 36.8198},
  {534.698, 28.9439, 36.7701},
  {543.346, 29.9487, 36.6969},
  {552.134, 30.9003, 36.6238},
  {561.065, 31.8344, 36.5509},
  {570.139, 32.7962, 36.4781},
  {579.361, 33.8521, 36.257},
  {588.732, 34.7681, 35.8649},
  {598.254, 35.5303, 35.4769},
  {607.93, 36.2207, 35.0932},
  {617.763, 36.8617, 34.7002},
  {627.755, 37.42, 34.2844},
  {637.908, 37.8729, 33.8737},
  {648.226, 38.1981, 33.5221},
  {658.711, 38.5267, 33.294},
  {669.365, 38.848, 33.0786},
  {680.191, 39.1292, 32.8968},
  {691.193, 39.3636, 32.7478},
  {702.372, 39.4881, 32.6614},
  {713.733, 39.577, 32.8294},
  {725.277, 39.873, 33.1766},
  {737.008, 40.327, 33.5274},
  {748.928, 40.9084, 33.8819},
  {761.042, 41.6486, 34.2312},
  {773.351, 42.6107, 34.4875},
  {785.859, 43.7001, 34.4328},
  {798.57, 44.5834, 34.1208},
  {811.486, 45.3299, 33.8119},
  {824.611, 46.0085, 33.5057},
  {837.949, 46.6466, 33.2022},
  {851.502, 47.2551, 32.8852},
  {865.274, 47.8317, 32.5504},
  {879.269, 48.3569, 32.2006},
  {893.491, 48.8358, 31.8547},
  {907.943, 49.2663, 31.5123},
  {922.628, 49.6009, 31.1735},
  {937.551, 49.927, 31.0086},
  {952.715, 50.2952, 30.8906},
  {968.124, 50.7455, 30.8134},
  {983.783, 51.2693, 30.7363},
  {999.695, 51.8785, 30.6289},
  {1015.86, 52.6224, 30.4894},
  {1032.29, 53.3038, 30.0388},
  {1048.99, 53.8761, 29.5846},
  {1065.96, 54.3682, 29.129},
  {1083.2, 54.813, 28.6797},
  {1100.72, 55.2218, 28.2371},
  {1118.52, 55.601, 27.801},
  {1136.61, 55.9547, 27.371},
  {1155, 56.286, 26.9475},
  {1173.68, 56.5974, 26.5303},
  {1192.66, 56.8904, 26.1191},
  {1211.95, 57.167, 25.7138},
  {1231.55, 57.4281, 25.3143},
  {1251.47, 57.6742, 24.9202},
  {1271.72, 57.9065, 24.5319},
  {1292.29, 58.1258, 24.1495},
  {1313.19, 58.333, 23.7726},
  {1334.43, 58.5289, 23.4011},
  {1356.01, 58.7137, 23.0347},
  {1377.94, 58.8879, 22.6736},
  {1400.23, 59.0524, 22.3178},
  {1422.88, 59.2084, 21.9672},
  {1445.89, 59.3573, 21.6213},
  {1469.28, 59.502, 21.277},
  {1493.04, 59.6434, 20.9325},
  {1517.19, 59.7692, 20.5721},
  {1541.73, 59.8751, 20.2146},
  {1566.67, 59.9635, 19.8626},
  {1592.01, 60.0377, 19.5159},
  {1617.76, 60.0984, 19.1751},
  {1643.92, 60.1471, 18.8402},
  {1670.51, 60.184, 18.5102},
  {1697.53, 60.2093, 18.1855},
  {1724.99, 60.2303, 17.866},
  {1752.89, 60.2379, 17.5376},
  {1781.24, 60.2257, 17.2103},
  {1810.05, 60.1944, 16.8856},
  {1839.32, 60.1435, 16.566},
  {1869.07, 60.0744, 16.2521},
  {1899.3, 59.9869, 15.944},
  {1930.02, 59.8809, 15.6417},
  {1961.24, 59.7556, 15.3446},
  {1992.96, 59.6066, 15.0522},
  {2025.2, 59.4467, 14.7741},
  {2057.95, 59.2648, 14.4832},
  {2091.24, 59.0486, 14.1946},
  {2125.06, 58.8002, 13.9102},
  {2159.43, 58.5156, 13.6284},
  {2194.36, 58.1909, 13.3511},
  {2229.85, 57.8201, 13.0765},
  {2265.92, 57.3956, 12.8056},
  {2302.57, 56.9078, 12.5382},
  {2339.81, 56.344, 12.2743},
  {2377.66, 55.6866, 12.014},
  {2416.11, 54.9108, 11.757},
  {2455.19, 53.9795, 11.5034},
  {2494.9, 52.8351, 11.2535},
  {2535.26, 51.3782, 11.0069},
  {2576.26, 49.4135, 10.7638},
  {2617.93, 46.4589, 10.5242},
  {2660.27, 40.4245, 10.2882},
  {2682.9, 11.5045, 10.1658},
  {2683.1, 11.5085, 26.8291},
  {2703.3, 39.2086, 26.5271},
  {2747.03, 43.59, 25.8925},
  {2791.46, 39.4607, 25.2737},
  {2797.9, 24.754, 25.186},
  {2798.1, 24.7677, 36.5361},
  {2836.61, 48.2036, 35.7868},
  {2882.49, 52.6811, 34.9267},
  {2929.11, 55.5306, 34.087},
  {2976.48, 57.6344, 33.269},
  {3024.63, 59.2715, 32.471},
  {3073.55, 60.5651, 31.6942},
  {3123.26, 61.5688, 30.9367},
  {3173.78, 62.2817, 30.1984},
  {3225.11, 62.6053, 29.4775},
  {3277.27, 61.9786, 28.7754},
  {3301.8, 54.6179, 28.4548},
  {3302, 54.6305, 33.0509},
  {3330.28, 63.0588, 32.6336},
  {3384.15, 65.3936, 31.863},
  {3438.88, 66.8185, 31.1118},
  {3494.5, 67.909, 30.3785},
  {3551.02, 68.7983, 29.6648},
  {3608.46, 69.5384, 28.9684},
  {3666.82, 70.1482, 28.289},
  {3726.13, 70.6219, 27.6263},
  {3786.4, 70.8995, 26.981},
  {3847.64, 70.2648, 26.3516},
  {3854, 68.2326, 26.2876},
  {3854.2, 68.2397, 27.8834},
  {3909.87, 71.8501, 27.2931},
  {3973.11, 72.64, 26.6474},
  {4037.38, 73.1707, 26.0168},
  {4102.68, 74.1366, 25.4011},
  {4149.3, 72.2509, 24.9761},
  {4149.5, 72.2586, 26.0075},
  {4169.03, 73.6831, 25.8385},
  {4236.46, 74.5992, 25.2697},
  {4304.98, 75.2045, 24.7094},
  {4374.62, 75.7079, 24.1576},
  {4445.37, 76.1492, 23.6172},
  {4517.27, 76.5459, 23.0865},
  {4590.33, 76.9068, 22.5654},
  {4664.58, 77.2366, 22.0529},
  {4740.03, 77.5381, 21.5497},
  {4816.69, 77.8151, 21.0566},
  {4894.6, 78.0704, 20.573},
  {4973.77, 78.3056, 20.0985},
  {5054.21, 78.5221, 19.6335},
  {5135.96, 78.7215, 19.1776},
  {5219.03, 78.9051, 18.7307},
  {5303.44, 79.0737, 18.2927},
  {5389.22, 79.2283, 17.8637},
  {5476.39, 79.37, 17.4435},
  {5564.97, 79.4994, 17.032},
  {5654.98, 79.6172, 16.629},
  {5746.44, 79.7242, 16.2346},
  {5839.39, 79.8208, 15.8486},
  {5933.83, 79.9079, 15.4709},
  {6029.81, 79.9859, 15.1013},
  {6127.33, 80.0551, 14.7396},
  {6226.44, 80.116, 14.3858},
  {6327.15, 80.1693, 14.0401},
  {6429.48, 80.2153, 13.7017},
  {6533.48, 80.254, 13.3709},
  {6639.15, 80.2861, 13.0476},
  {6746.54, 80.3117, 12.7316},
  {6855.65, 80.3314, 12.4229},
  {6966.54, 80.3454, 12.1211},
  {7079.22, 80.3537, 11.8259},
  {7193.72, 80.3567, 11.538},
  {7310.07, 80.3548, 11.2564},
  {7428.31, 80.3478, 10.9814},
  {7548.45, 80.336, 10.713},
  {7670.54, 80.3201, 10.4509},
  {7794.61, 80.2997, 10.1948},
  {7920.68, 80.275, 9.94471},
  {8048.79, 80.2462, 9.70051},
  {8178.98, 80.2133, 9.46218},
  {8311.26, 80.1766, 9.22946},
  {8445.69, 80.136, 9.00226},
  {8582.29, 80.0916, 8.78071},
  {8721.11, 80.0435, 8.56415},
  {8862.16, 79.9916, 8.35312},
  {9005.5, 79.9361, 8.14686},
  {9151.16, 79.8767, 7.94577},
  {9299.17, 79.8139, 7.74959},
  {9449.58, 79.7475, 7.558},
  {9602.42, 79.6775, 7.37103},
  {9757.73, 79.604, 7.18894},
  {9915.55, 79.5339, 7.01103},
  {10075.9, 79.4592, 6.82141},
  {10238.9, 79.365, 6.63738},
  {10404.5, 79.2631, 6.45881},
  {10572.8, 79.1523, 6.28558},
  {10743.8, 79.0327, 6.11759},
  {10917.6, 78.9037, 5.95472},
  {11094.2, 78.7646, 5.79685},
  {11273.6, 78.6143, 5.64388},
  {11455.9, 78.4514, 5.49569},
  {11641.2, 78.274, 5.35217},
  {11829.5, 78.0796, 5.2132},
  {12020.8, 77.8646, 5.07869},
  {12215.3, 77.6243, 4.94851},
  {12412.8, 77.3516, 4.82258},
  {12613.6, 77.0362, 4.70077},
  {12817.6, 76.6614, 4.583},
  {13025, 76.198, 4.46915},
  {13235.6, 75.5884, 4.35912},
  {13449.7, 74.6896, 4.25282},
  {13667.2, 72.9266, 4.15014},
  {13813.7, 58.4901, 4.08331},
  {13813.9, 58.4906, 10.3379},
  {13888.3, 71.7171, 10.2428},
  {14112.9, 74.5226, 9.96377},
  {14341.2, 75.6213, 9.6924},
  {14573.1, 76.2694, 9.42841},
  {14808.9, 76.6856, 9.17163},
  {15048.4, 76.944, 8.92183},
  {15291.8, 77.0699, 8.67885},
  {15539.1, 77.0567, 8.4425},
  {15790.4, 76.8501, 8.21257},
  {16045.8, 76.2243, 7.98892},
  {16244.2, 68.9899, 7.82179},
  {16244.4, 68.9907, 10.9274},
  {16305.4, 75.185, 10.8688},
  {16569.1, 76.8432, 10.6218},
  {16837.1, 77.1094, 10.3803},
  {16939.2, 74.4097, 10.2908},
  {16939.4, 74.4102, 11.621},
  {17109.4, 77.9036, 11.4616},
  {17386.1, 78.7621, 11.2101},
  {17667.4, 79.3429, 10.9617},
  {17953.1, 79.8037, 10.7164},
  {18243.5, 80.1906, 10.4745},
  {18538.6, 80.5249, 10.236},
  {18838.4, 80.8191, 10.0009},
  {19143.1, 81.0811, 9.76948},
  {19452.7, 81.3161, 9.54166},
  {19767.4, 81.5284, 9.31752},
  {20087.1, 81.7209, 9.09713},
  {20412, 81.8961, 8.8805},
  {20742.1, 82.0559, 8.66767},
  {21077.6, 82.2019, 8.45865},
  {21418.5, 82.3354, 8.25346},
  {21765, 82.4576, 8.05211},
  {22117, 82.5694, 7.8546},
  {22474.7, 82.6718, 7.66092},
  {22838.2, 82.7655, 7.47105},
  {23207.6, 82.8511, 7.28499},
  {23583, 82.9293, 7.10271},
  {23964.4, 83.0006, 6.92419},
  {24352, 83.0654, 6.74939},
  {24745.9, 83.1243, 6.5783},
  {25146.2, 83.1776, 6.41087},
  {25552.9, 83.2258, 6.24708},
  {25966.2, 83.2691, 6.08687},
  {26386.1, 83.3078, 5.93021},
  {26812.9, 83.3423, 5.77705},
  {27246.6, 83.3728, 5.62735},
  {27687.3, 83.3997, 5.48107},
  {28135.1, 83.423, 5.33814},
  {28590.2, 83.443, 5.19853},
  {29052.6, 83.46, 5.06218},
  {29522.5, 83.4742, 4.92904},
  {30000, 83.4856, 4.79906}
};

