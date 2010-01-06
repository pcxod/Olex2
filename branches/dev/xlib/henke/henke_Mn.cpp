#include "../henke.h"
#define NOVAL cm_Anomalous_Henke::Undefined
static const cm_Anomalous_Henke XlibObject(_cm_henke_Mn)[] = {
  {10, NOVAL, 1.8899},
  {10.1617, NOVAL, 1.92644},
  {10.3261, NOVAL, 1.96368},
  {10.4931, NOVAL, 2.00165},
  {10.6628, NOVAL, 2.04035},
  {10.8353, NOVAL, 2.0798},
  {11.0106, NOVAL, 2.12001},
  {11.1886, NOVAL, 2.161},
  {11.3696, NOVAL, 2.20278},
  {11.5535, NOVAL, 2.24537},
  {11.7404, NOVAL, 2.28879},
  {11.9303, NOVAL, 2.33304},
  {12.1232, NOVAL, 2.37676},
  {12.3193, NOVAL, 2.4213},
  {12.5186, NOVAL, 2.46667},
  {12.721, NOVAL, 2.51289},
  {12.9268, NOVAL, 2.55998},
  {13.1359, NOVAL, 2.60796},
  {13.3483, NOVAL, 2.65683},
  {13.5642, NOVAL, 2.70662},
  {13.7836, NOVAL, 2.75734},
  {14.0066, NOVAL, 2.80901},
  {14.2331, NOVAL, 2.86165},
  {14.4633, NOVAL, 2.91527},
  {14.6973, NOVAL, 2.96991},
  {14.935, NOVAL, 3.02556},
  {15.1765, NOVAL, 3.08226},
  {15.422, NOVAL, 3.14002},
  {15.6714, NOVAL, 3.19886},
  {15.9249, NOVAL, 3.2588},
  {16.1825, NOVAL, 3.31987},
  {16.4442, NOVAL, 3.38209},
  {16.7102, NOVAL, 3.44217},
  {16.9805, NOVAL, 3.48884},
  {17.2551, NOVAL, 3.53615},
  {17.5342, NOVAL, 3.58409},
  {17.8178, NOVAL, 3.63268},
  {18.106, NOVAL, 3.68194},
  {18.3989, NOVAL, 3.73186},
  {18.6964, NOVAL, 3.78246},
  {18.9988, NOVAL, 3.83374},
  {19.3061, NOVAL, 3.85544},
  {19.6184, NOVAL, 3.87253},
  {19.9357, NOVAL, 3.8897},
  {20.2582, NOVAL, 3.90694},
  {20.5858, NOVAL, 3.92427},
  {20.9188, NOVAL, 3.94166},
  {21.2571, NOVAL, 3.95914},
  {21.6009, NOVAL, 3.97669},
  {21.9503, NOVAL, 3.99432},
  {22.3053, NOVAL, 4.01203},
  {22.6661, NOVAL, 4.0003},
  {23.0327, NOVAL, 3.97304},
  {23.4053, NOVAL, 3.94596},
  {23.7838, NOVAL, 3.91906},
  {24.1685, NOVAL, 3.89235},
  {24.5594, NOVAL, 3.86582},
  {24.9566, NOVAL, 3.83947},
  {25.3603, NOVAL, 3.8133},
  {25.7705, NOVAL, 3.78323},
  {26.1873, NOVAL, 3.72987},
  {26.6109, NOVAL, 3.67726},
  {27.0413, NOVAL, 3.62539},
  {27.4786, NOVAL, 3.57425},
  {27.9231, NOVAL, 3.52383},
  {28.3747, NOVAL, 3.47413},
  {28.8337, NOVAL, 3.42513},
  {29.3, 6.5024, 3.37693},
  {29.7739, 6.52171, 3.32983},
  {30.2555, 6.53714, 3.28339},
  {30.7449, 6.54862, 3.23759},
  {31.2421, 6.55611, 3.19243},
  {31.7475, 6.55952, 3.14791},
  {32.2609, 6.5587, 3.104},
  {32.7827, 6.55342, 3.0607},
  {33.313, 6.54314, 3.01801},
  {33.8518, 6.52639, 2.9769},
  {34.3993, 6.50655, 2.94108},
  {34.9557, 6.48445, 2.90568},
  {35.5211, 6.459, 2.87072},
  {36.0956, 6.42978, 2.83618},
  {36.6794, 6.39724, 2.80205},
  {37.2727, 6.36467, 2.76833},
  {37.8755, 6.32496, 2.71646},
  {38.4882, 6.26729, 2.66435},
  {39.1107, 6.19462, 2.61324},
  {39.7432, 6.10733, 2.56311},
  {40.3861, 6.00368, 2.51394},
  {41.0393, 5.88056, 2.46597},
  {41.7031, 5.73569, 2.4214},
  {42.3776, 5.56396, 2.37764},
  {43.063, 5.35416, 2.33467},
  {43.7595, 5.07923, 2.30245},
  {44.4673, 4.7717, 2.33906},
  {45.1865, 4.41786, 2.37625},
  {45.9174, 3.93173, 2.41403},
  {46.66, 3.29552, 2.7447},
  {47.4147, 2.77858, 3.25644},
  {48.1816, 2.27583, 3.86359},
  {48.9609, 1.635, 4.58393},
  {49.7528, 1.15257, 6.20992},
  {50.5576, 2.00701, 8.3674},
  {51.3753, 3.2773, 8.94377},
  {52.2062, 4.50051, 9.43837},
  {53.0506, 5.54117, 9.46},
  {53.9087, 6.51927, 9.3757},
  {54.7806, 7.24311, 9.05133},
  {55.6667, 7.77471, 8.73819},
  {56.567, 8.20129, 8.43393},
  {57.482, 8.54735, 8.1393},
  {58.4117, 8.82762, 7.85497},
  {59.3564, 9.04566, 7.58057},
  {60.3165, 9.19693, 7.35293},
  {61.2921, 9.35392, 7.16484},
  {62.2834, 9.50102, 6.98157},
  {63.2908, 9.63326, 6.80299},
  {64.3145, 9.74965, 6.62897},
  {65.3547, 9.84953, 6.4594},
  {66.4118, 9.92971, 6.29418},
  {67.4859, 9.99262, 6.14892},
  {68.5775, 10.0529, 6.01221},
  {69.6867, 10.1062, 5.87852},
  {70.8138, 10.149, 5.74781},
  {71.9591, 10.1789, 5.62},
  {73.123, 10.1882, 5.49504},
  {74.3057, 10.1626, 5.40548},
  {75.5076, 10.163, 5.34635},
  {76.7289, 10.1765, 5.28786},
  {77.9699, 10.1918, 5.23001},
  {79.231, 10.2061, 5.1728},
  {80.5125, 10.2159, 5.1162},
  {81.8147, 10.2108, 5.06786},
  {83.138, 10.2151, 5.03821},
  {84.4827, 10.227, 5.00872},
  {85.8491, 10.2406, 4.9794},
  {87.2377, 10.2544, 4.95026},
  {88.6487, 10.2672, 4.92129},
  {90.0825, 10.2779, 4.89248},
  {91.5395, 10.2851, 4.86385},
  {93.0201, 10.2872, 4.83538},
  {94.5246, 10.2798, 4.80708},
  {96.0535, 10.2462, 4.77895},
  {97.6071, 10.2127, 4.80833},
  {99.1858, 10.205, 4.84354},
  {100.79, 10.2096, 4.87899},
  {102.42, 10.2222, 4.91472},
  {104.077, 10.2407, 4.9507},
  {105.76, 10.264, 4.98695},
  {107.471, 10.2914, 5.02345},
  {109.209, 10.3222, 5.06023},
  {110.975, 10.3557, 5.09728},
  {112.77, 10.3922, 5.13662},
  {114.594, 10.4327, 5.17671},
  {116.448, 10.477, 5.21712},
  {118.331, 10.5246, 5.25785},
  {120.245, 10.5756, 5.29889},
  {122.19, 10.63, 5.34025},
  {124.166, 10.688, 5.38194},
  {126.175, 10.7501, 5.42395},
  {128.215, 10.8173, 5.46629},
  {130.289, 10.8934, 5.5066},
  {132.397, 10.9691, 5.53684},
  {134.538, 11.044, 5.56727},
  {136.714, 11.12, 5.59785},
  {138.925, 11.1983, 5.6286},
  {141.172, 11.2796, 5.65952},
  {143.456, 11.365, 5.69061},
  {145.776, 11.4562, 5.72187},
  {148.134, 11.5578, 5.75331},
  {150.53, 11.682, 5.76483},
  {152.964, 11.7888, 5.75371},
  {155.439, 11.8822, 5.74262},
  {157.953, 11.9713, 5.73156},
  {160.507, 12.0571, 5.72051},
  {163.103, 12.1406, 5.70948},
  {165.742, 12.2225, 5.69848},
  {168.422, 12.3032, 5.6875},
  {171.146, 12.3832, 5.67653},
  {173.915, 12.4631, 5.66559},
  {176.727, 12.5456, 5.65301},
  {179.586, 12.6252, 5.63644},
  {182.491, 12.702, 5.61993},
  {185.442, 12.7777, 5.60345},
  {188.442, 12.8524, 5.58703},
  {191.489, 12.9267, 5.57066},
  {194.587, 13.0006, 5.55434},
  {197.734, 13.0749, 5.53806},
  {200.932, 13.1493, 5.52017},
  {204.182, 13.2229, 5.50172},
  {207.485, 13.2958, 5.48333},
  {210.84, 13.3688, 5.465},
  {214.251, 13.4421, 5.44674},
  {217.716, 13.516, 5.42853},
  {221.237, 13.5909, 5.41039},
  {224.816, 13.6674, 5.3923},
  {228.452, 13.7462, 5.37429},
  {232.147, 13.8285, 5.35632},
  {235.902, 13.9169, 5.33842},
  {239.717, 14.0201, 5.32057},
  {243.595, 14.1372, 5.26706},
  {247.535, 14.232, 5.19443},
  {251.538, 14.3082, 5.1228},
  {255.607, 14.3761, 5.05216},
  {259.741, 14.4375, 4.98249},
  {263.942, 14.4935, 4.91378},
  {268.211, 14.545, 4.84602},
  {272.549, 14.5924, 4.7792},
  {276.957, 14.6364, 4.71329},
  {281.437, 14.6775, 4.6483},
  {285.989, 14.7164, 4.5842},
  {290.615, 14.7552, 4.52098},
  {295.315, 14.7921, 4.45086},
  {300.092, 14.8219, 4.37922},
  {304.945, 14.8455, 4.30873},
  {309.878, 14.8647, 4.23938},
  {314.89, 14.8799, 4.17114},
  {319.983, 14.8912, 4.104},
  {325.158, 14.899, 4.03795},
  {330.418, 14.9032, 3.97296},
  {335.762, 14.9039, 3.90901},
  {341.192, 14.9012, 3.84609},
  {346.711, 14.8952, 3.78419},
  {352.319, 14.8859, 3.72328},
  {358.017, 14.8731, 3.66298},
  {363.808, 14.8567, 3.6035},
  {369.692, 14.8364, 3.54499},
  {375.672, 14.8125, 3.48742},
  {381.748, 14.7847, 3.4308},
  {387.922, 14.7531, 3.37508},
  {394.197, 14.7176, 3.32028},
  {400.573, 14.6781, 3.26636},
  {407.052, 14.6348, 3.21332},
  {413.635, 14.588, 3.16114},
  {420.326, 14.543, 3.10587},
  {427.124, 14.4869, 3.04259},
  {434.032, 14.4186, 2.98059},
  {441.052, 14.3412, 2.91986},
  {448.186, 14.2546, 2.86037},
  {455.435, 14.1583, 2.80209},
  {462.802, 14.0517, 2.745},
  {470.287, 13.9336, 2.68907},
  {477.894, 13.801, 2.63428},
  {485.623, 13.6546, 2.586},
  {493.478, 13.4977, 2.54166},
  {501.459, 13.3272, 2.49807},
  {509.57, 13.1399, 2.45524},
  {517.812, 12.9329, 2.41314},
  {526.187, 12.7025, 2.37176},
  {534.698, 12.4439, 2.33115},
  {543.346, 12.1526, 2.29292},
  {552.134, 11.822, 2.25534},
  {561.065, 11.4414, 2.21646},
  {570.139, 10.995, 2.17739},
  {579.361, 10.462, 2.13934},
  {588.732, 9.811, 2.10212},
  {598.254, 8.98789, 2.06565},
  {607.93, 7.8916, 2.02981},
  {617.763, 6.29719, 1.9946},
  {627.755, 3.50844, 1.95999},
  {637.908, -8.42761, 1.92599},
  {638.6, -18.0574, 1.92371},
  {638.8, -18.0426, 16.5984},
  {648.226, 3.56736, 16.3194},
  {658.711, 7.25819, 16.0192},
  {669.365, 9.41955, 15.7245},
  {680.191, 10.9601, 15.4352},
  {691.193, 12.155, 15.1512},
  {702.372, 13.1261, 14.8725},
  {713.733, 13.9327, 14.587},
  {725.277, 14.6093, 14.2976},
  {737.008, 15.0046, 13.967},
  {748.928, 15.2444, 13.9671},
  {761.042, 15.6958, 14.2068},
  {773.351, 16.3817, 14.4506},
  {785.859, 17.2721, 14.3998},
  {798.57, 18.0296, 14.105},
  {811.486, 18.5918, 13.7795},
  {824.611, 19.0745, 13.4617},
  {837.949, 19.4896, 13.1513},
  {851.502, 19.8572, 12.861},
  {865.274, 20.1883, 12.5903},
  {879.269, 20.4999, 12.3369},
  {893.491, 20.7973, 12.0887},
  {907.943, 21.0774, 11.8456},
  {922.628, 21.3469, 11.6073},
  {937.551, 21.6002, 11.3636},
  {952.715, 21.8356, 11.1225},
  {968.124, 22.0546, 10.8843},
  {983.783, 22.2591, 10.6513},
  {999.695, 22.4516, 10.4227},
  {1015.86, 22.635, 10.1985},
  {1032.29, 22.808, 9.97462},
  {1048.99, 22.9709, 9.75224},
  {1065.96, 23.1213, 9.5312},
  {1083.2, 23.2594, 9.31505},
  {1100.72, 23.3881, 9.10383},
  {1118.52, 23.5081, 8.89743},
  {1136.61, 23.6204, 8.69577},
  {1155, 23.7256, 8.49864},
  {1173.68, 23.8242, 8.30594},
  {1192.66, 23.9168, 8.11769},
  {1211.95, 24.004, 7.9337},
  {1231.55, 24.0872, 7.75385},
  {1251.47, 24.166, 7.57626},
  {1271.72, 24.2403, 7.40089},
  {1292.29, 24.3087, 7.22735},
  {1313.19, 24.3709, 7.05785},
  {1334.43, 24.4285, 6.89231},
  {1356.01, 24.482, 6.73065},
  {1377.94, 24.5316, 6.57273},
  {1400.23, 24.5777, 6.41852},
  {1422.88, 24.6208, 6.26798},
  {1445.89, 24.6616, 6.12099},
  {1469.28, 24.7002, 5.97676},
  {1493.04, 24.738, 5.83489},
  {1517.19, 24.7721, 5.69216},
  {1541.73, 24.8012, 5.55236},
  {1566.67, 24.8268, 5.41601},
  {1592.01, 24.8496, 5.28314},
  {1617.76, 24.87, 5.15347},
  {1643.92, 24.8881, 5.02684},
  {1670.51, 24.9043, 4.90343},
  {1697.53, 24.9191, 4.7831},
  {1724.99, 24.9338, 4.66575},
  {1752.89, 24.947, 4.54758},
  {1781.24, 24.9573, 4.43117},
  {1810.05, 24.9644, 4.31685},
  {1839.32, 24.969, 4.20548},
  {1869.07, 24.9715, 4.09702},
  {1899.3, 24.9723, 3.99146},
  {1930.02, 24.9718, 3.88854},
  {1961.24, 24.97, 3.78823},
  {1992.96, 24.9662, 3.69045},
  {2025.2, 24.9631, 3.59707},
  {2057.95, 24.9596, 3.50268},
  {2091.24, 24.9542, 3.41037},
  {2125.06, 24.9471, 3.32011},
  {2159.43, 24.9388, 3.23221},
  {2194.36, 24.9296, 3.1464},
  {2229.85, 24.9194, 3.06252},
  {2265.92, 24.9081, 2.9807},
  {2302.57, 24.896, 2.90088},
  {2339.81, 24.8831, 2.82302},
  {2377.66, 24.8693, 2.74705},
  {2416.11, 24.8548, 2.673},
  {2455.19, 24.8397, 2.60077},
  {2494.9, 24.8238, 2.53038},
  {2535.26, 24.8074, 2.46175},
  {2576.26, 24.7903, 2.39486},
  {2617.93, 24.7728, 2.32969},
  {2660.27, 24.7547, 2.26618},
  {2703.3, 24.7361, 2.20433},
  {2747.03, 24.717, 2.14406},
  {2791.46, 24.6975, 2.08536},
  {2836.61, 24.6776, 2.0282},
  {2882.49, 24.6573, 1.97256},
  {2929.11, 24.6366, 1.91837},
  {2976.48, 24.6155, 1.86562},
  {3024.63, 24.5941, 1.81427},
  {3073.55, 24.5724, 1.76427},
  {3123.26, 24.5503, 1.71563},
  {3173.78, 24.5278, 1.66829},
  {3225.11, 24.505, 1.62224},
  {3277.27, 24.482, 1.57744},
  {3330.28, 24.4585, 1.53385},
  {3384.15, 24.4348, 1.49147},
  {3438.88, 24.4107, 1.45022},
  {3494.5, 24.3863, 1.41011},
  {3551.02, 24.3616, 1.3711},
  {3608.46, 24.3364, 1.33316},
  {3666.82, 24.3109, 1.29628},
  {3726.13, 24.285, 1.26044},
  {3786.4, 24.2586, 1.22557},
  {3847.64, 24.2318, 1.19169},
  {3909.87, 24.2045, 1.15878},
  {3973.11, 24.1767, 1.12676},
  {4037.38, 24.1483, 1.09562},
  {4102.68, 24.1192, 1.0654},
  {4169.03, 24.0895, 1.03601},
  {4236.46, 24.059, 1.00747},
  {4304.98, 24.0278, 0.979721},
  {4374.62, 23.9956, 0.952745},
  {4445.37, 23.9624, 0.926585},
  {4517.27, 23.9281, 0.901138},
  {4590.33, 23.8925, 0.876412},
  {4664.58, 23.8556, 0.852412},
  {4740.03, 23.8171, 0.829086},
  {4816.69, 23.7768, 0.806453},
  {4894.6, 23.7345, 0.784456},
  {4973.77, 23.69, 0.763123},
  {5054.21, 23.6428, 0.742345},
  {5135.96, 23.5925, 0.722213},
  {5219.03, 23.5388, 0.702659},
  {5303.44, 23.481, 0.683674},
  {5389.22, 23.4182, 0.665236},
  {5476.39, 23.3496, 0.647339},
  {5564.97, 23.2739, 0.629964},
  {5654.98, 23.1894, 0.613093},
  {5746.44, 23.0938, 0.596717},
  {5839.39, 22.984, 0.580824},
  {5933.83, 22.8551, 0.565389},
  {6029.81, 22.6997, 0.550414},
  {6127.33, 22.5053, 0.535881},
  {6226.44, 22.2484, 0.521775},
  {6327.15, 21.8761, 0.508081},
  {6429.48, 21.2225, 0.494788},
  {6533.48, 18.0825, 0.481891},
  {6538.9, 13.7573, 0.481234},
  {6539.1, 13.7578, 3.8759},
  {6639.15, 21.3097, 3.7917},
  {6746.54, 22.1537, 3.70472},
  {6855.65, 22.6533, 3.61897},
  {6966.54, 23.0114, 3.53438},
  {7079.22, 23.2908, 3.45107},
  {7193.72, 23.5195, 3.36899},
  {7310.07, 23.7123, 3.28831},
  {7428.31, 23.8785, 3.20892},
  {7548.45, 24.0236, 3.13076},
  {7670.54, 24.1519, 3.05404},
  {7794.61, 24.2663, 2.97871},
  {7920.68, 24.369, 2.90471},
  {8048.79, 24.4617, 2.83205},
  {8178.98, 24.5456, 2.76082},
  {8311.26, 24.6219, 2.69094},
  {8445.69, 24.6915, 2.62248},
  {8582.29, 24.755, 2.55531},
  {8721.11, 24.8132, 2.48948},
  {8862.16, 24.8664, 2.42505},
  {9005.5, 24.9152, 2.36203},
  {9151.16, 24.96, 2.30031},
  {9299.17, 25.0011, 2.23986},
  {9449.58, 25.0389, 2.18076},
  {9602.42, 25.0735, 2.12295},
  {9757.73, 25.1052, 2.06653},
  {9915.55, 25.1344, 2.01123},
  {10075.9, 25.1611, 1.95725},
  {10238.9, 25.1855, 1.90452},
  {10404.5, 25.2078, 1.853},
  {10572.8, 25.2281, 1.80269},
  {10743.8, 25.2466, 1.75357},
  {10917.6, 25.2634, 1.70562},
  {11094.2, 25.2786, 1.65882},
  {11273.6, 25.2924, 1.61315},
  {11455.9, 25.3048, 1.56859},
  {11641.2, 25.3158, 1.52512},
  {11829.5, 25.3257, 1.48273},
  {12020.8, 25.3345, 1.44138},
  {12215.3, 25.3422, 1.40108},
  {12412.8, 25.3489, 1.36178},
  {12613.6, 25.3547, 1.32349},
  {12817.6, 25.3597, 1.28616},
  {13025, 25.3638, 1.2498},
  {13235.6, 25.3672, 1.21436},
  {13449.7, 25.3699, 1.17985},
  {13667.2, 25.3719, 1.14623},
  {13888.3, 25.3734, 1.11349},
  {14112.9, 25.3742, 1.08161},
  {14341.2, 25.3746, 1.05057},
  {14573.1, 25.3744, 1.02036},
  {14808.9, 25.3738, 0.990942},
  {15048.4, 25.3728, 0.962313},
  {15291.8, 25.3713, 0.934454},
  {15539.1, 25.3695, 0.907344},
  {15790.4, 25.3674, 0.880966},
  {16045.8, 25.3649, 0.855305},
  {16305.4, 25.3622, 0.830341},
  {16569.1, 25.3591, 0.80606},
  {16837.1, 25.3559, 0.782444},
  {17109.4, 25.3524, 0.759478},
  {17386.1, 25.3486, 0.737146},
  {17667.4, 25.3447, 0.715431},
  {17953.1, 25.3407, 0.69432},
  {18243.5, 25.3364, 0.673796},
  {18538.6, 25.3321, 0.653846},
  {18838.4, 25.3276, 0.634455},
  {19143.1, 25.3229, 0.615608},
  {19452.7, 25.3182, 0.597292},
  {19767.4, 25.3134, 0.579493},
  {20087.1, 25.3085, 0.562198},
  {20412, 25.3036, 0.545393},
  {20742.1, 25.2985, 0.529067},
  {21077.6, 25.2935, 0.513206},
  {21418.5, 25.2883, 0.497799},
  {21765, 25.2832, 0.482833},
  {22117, 25.278, 0.468296},
  {22474.7, 25.2728, 0.454179},
  {22838.2, 25.2676, 0.440468},
  {23207.6, 25.2624, 0.427153},
  {23583, 25.2572, 0.414224},
  {23964.4, 25.252, 0.40167},
  {24352, 25.2468, 0.389481},
  {24745.9, 25.2416, 0.377647},
  {25146.2, 25.2364, 0.366158},
  {25552.9, 25.2313, 0.355006},
  {25966.2, 25.2262, 0.34418},
  {26386.1, 25.2211, 0.333671},
  {26812.9, 25.216, 0.323471},
  {27246.6, 25.211, 0.313572},
  {27687.3, 25.206, 0.303965},
  {28135.1, 25.2011, 0.294641},
  {28590.2, 25.1962, 0.285593},
  {29052.6, 25.1915, 0.276814},
  {29522.5, 25.1875, 0.268295},
  {30000, 25.1795, 0.260029}
};

