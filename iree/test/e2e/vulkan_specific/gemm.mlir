func @gemm() attributes { iree.module.export } {
  %0 = iree.unfoldable_constant dense<
      [[0.        , 0.00515464, 0.01030928, 0.01546392, 0.02061856,
        0.0257732 , 0.03092784, 0.03608247, 0.04123711, 0.04639175,
        0.05154639, 0.05670103, 0.06185567, 0.06701031, 0.07216495],
       [0.07731959, 0.08247423, 0.08762887, 0.09278351, 0.09793814,
        0.10309278, 0.10824742, 0.11340206, 0.1185567 , 0.12371134,
        0.12886598, 0.13402062, 0.13917526, 0.1443299 , 0.14948454],
       [0.15463918, 0.15979381, 0.16494845, 0.17010309, 0.17525773,
        0.18041237, 0.18556701, 0.19072165, 0.19587629, 0.20103093,
        0.20618557, 0.21134021, 0.21649485, 0.22164948, 0.22680412],
       [0.23195876, 0.2371134 , 0.24226804, 0.24742268, 0.25257732,
        0.25773196, 0.2628866 , 0.26804124, 0.27319588, 0.27835052,
        0.28350515, 0.28865979, 0.29381443, 0.29896907, 0.30412371],
       [0.30927835, 0.31443299, 0.31958763, 0.32474227, 0.32989691,
        0.33505155, 0.34020619, 0.34536082, 0.35051546, 0.3556701 ,
        0.36082474, 0.36597938, 0.37113402, 0.37628866, 0.3814433 ],
       [0.38659794, 0.39175258, 0.39690722, 0.40206186, 0.40721649,
        0.41237113, 0.41752577, 0.42268041, 0.42783505, 0.43298969,
        0.43814433, 0.44329897, 0.44845361, 0.45360825, 0.45876289],
       [0.46391753, 0.46907216, 0.4742268 , 0.47938144, 0.48453608,
        0.48969072, 0.49484536, 0.5       , 0.50515464, 0.51030928,
        0.51546392, 0.52061856, 0.5257732 , 0.53092784, 0.53608247],
       [0.54123711, 0.54639175, 0.55154639, 0.55670103, 0.56185567,
        0.56701031, 0.57216495, 0.57731959, 0.58247423, 0.58762887,
        0.59278351, 0.59793814, 0.60309278, 0.60824742, 0.61340206],
       [0.6185567 , 0.62371134, 0.62886598, 0.63402062, 0.63917526,
        0.6443299 , 0.64948454, 0.65463918, 0.65979381, 0.66494845,
        0.67010309, 0.67525773, 0.68041237, 0.68556701, 0.69072165],
       [0.69587629, 0.70103093, 0.70618557, 0.71134021, 0.71649485,
        0.72164948, 0.72680412, 0.73195876, 0.7371134 , 0.74226804,
        0.74742268, 0.75257732, 0.75773196, 0.7628866 , 0.76804124],
       [0.77319588, 0.77835052, 0.78350515, 0.78865979, 0.79381443,
        0.79896907, 0.80412371, 0.80927835, 0.81443299, 0.81958763,
        0.82474227, 0.82989691, 0.83505155, 0.84020619, 0.84536082],
       [0.85051546, 0.8556701 , 0.86082474, 0.86597938, 0.87113402,
        0.87628866, 0.8814433 , 0.88659794, 0.89175258, 0.89690722,
        0.90206186, 0.90721649, 0.91237113, 0.91752577, 0.92268041],
       [0.92783505, 0.93298969, 0.93814433, 0.94329897, 0.94845361,
        0.95360825, 0.95876289, 0.96391753, 0.96907216, 0.9742268 ,
        0.97938144, 0.98453608, 0.98969072, 0.99484536, 1.        ]]>
    : tensor<13x15xf32>
  %1 = iree.unfoldable_constant dense<
       [[0.        , 0.00558659, 0.01117318, 0.01675978, 0.02234637,
        0.02793296, 0.03351955, 0.03910615, 0.04469274, 0.05027933,
        0.05586592, 0.06145251],
       [0.06703911, 0.0726257 , 0.07821229, 0.08379888, 0.08938547,
        0.09497207, 0.10055866, 0.10614525, 0.11173184, 0.11731844,
        0.12290503, 0.12849162],
       [0.13407821, 0.1396648 , 0.1452514 , 0.15083799, 0.15642458,
        0.16201117, 0.16759777, 0.17318436, 0.17877095, 0.18435754,
        0.18994413, 0.19553073],
       [0.20111732, 0.20670391, 0.2122905 , 0.21787709, 0.22346369,
        0.22905028, 0.23463687, 0.24022346, 0.24581006, 0.25139665,
        0.25698324, 0.26256983],
       [0.26815642, 0.27374302, 0.27932961, 0.2849162 , 0.29050279,
        0.29608939, 0.30167598, 0.30726257, 0.31284916, 0.31843575,
        0.32402235, 0.32960894],
       [0.33519553, 0.34078212, 0.34636872, 0.35195531, 0.3575419 ,
        0.36312849, 0.36871508, 0.37430168, 0.37988827, 0.38547486,
        0.39106145, 0.39664804],
       [0.40223464, 0.40782123, 0.41340782, 0.41899441, 0.42458101,
        0.4301676 , 0.43575419, 0.44134078, 0.44692737, 0.45251397,
        0.45810056, 0.46368715],
       [0.46927374, 0.47486034, 0.48044693, 0.48603352, 0.49162011,
        0.4972067 , 0.5027933 , 0.50837989, 0.51396648, 0.51955307,
        0.52513966, 0.53072626],
       [0.53631285, 0.54189944, 0.54748603, 0.55307263, 0.55865922,
        0.56424581, 0.5698324 , 0.57541899, 0.58100559, 0.58659218,
        0.59217877, 0.59776536],
       [0.60335196, 0.60893855, 0.61452514, 0.62011173, 0.62569832,
        0.63128492, 0.63687151, 0.6424581 , 0.64804469, 0.65363128,
        0.65921788, 0.66480447],
       [0.67039106, 0.67597765, 0.68156425, 0.68715084, 0.69273743,
        0.69832402, 0.70391061, 0.70949721, 0.7150838 , 0.72067039,
        0.72625698, 0.73184358],
       [0.73743017, 0.74301676, 0.74860335, 0.75418994, 0.75977654,
        0.76536313, 0.77094972, 0.77653631, 0.78212291, 0.7877095 ,
        0.79329609, 0.79888268],
       [0.80446927, 0.81005587, 0.81564246, 0.82122905, 0.82681564,
        0.83240223, 0.83798883, 0.84357542, 0.84916201, 0.8547486 ,
        0.8603352 , 0.86592179],
       [0.87150838, 0.87709497, 0.88268156, 0.88826816, 0.89385475,
        0.89944134, 0.90502793, 0.91061453, 0.91620112, 0.92178771,
        0.9273743 , 0.93296089],
       [0.93854749, 0.94413408, 0.94972067, 0.95530726, 0.96089385,
        0.96648045, 0.97206704, 0.97765363, 0.98324022, 0.98882682,
        0.99441341, 1.        ]]> : tensor<15x12xf32>
  %2 = "mhlo.dot"(%0, %1) : (tensor<13x15xf32>, tensor<15x12xf32>)
                            -> (tensor<13x12xf32>)
  check.expect_almost_eq_const(%2,
    dense<
      [[0.35074584, 0.35376951, 0.35679318, 0.35981685, 0.36284052,
        0.36586419, 0.36888787, 0.37191154, 0.37493521, 0.37795888,
        0.38098255, 0.38400622],
       [0.89500662, 0.90450959, 0.91401256, 0.92351552, 0.93301849,
        0.94252145, 0.95202442, 0.96152739, 0.97103035, 0.98053332,
        0.99003628, 0.99953925],
       [1.43926741, 1.45524967, 1.47123193, 1.48721419, 1.50319645,
        1.51917871, 1.53516097, 1.55114324, 1.5671255 , 1.58310776,
        1.59909002, 1.61507228],
       [1.98352819, 2.00598975, 2.0284513 , 2.05091286, 2.07337442,
        2.09583597, 2.11829753, 2.14075909, 2.16322064, 2.1856822 ,
        2.20814375, 2.23060531],
       [2.52778898, 2.55672983, 2.58567068, 2.61461153, 2.64355238,
        2.67249323, 2.70143408, 2.73037494, 2.75931579, 2.78825664,
        2.81719749, 2.84613834],
       [3.07204976, 3.10746991, 3.14289005, 3.1783102 , 3.21373035,
        3.24915049, 3.28457064, 3.31999079, 3.35541093, 3.39083108,
        3.42625122, 3.46167137],
       [3.61631055, 3.65820999, 3.70010943, 3.74200887, 3.78390831,
        3.82580775, 3.86770719, 3.90960663, 3.95150608, 3.99340552,
        4.03530496, 4.0772044 ],
       [4.16057133, 4.20895007, 4.2573288 , 4.30570754, 4.35408628,
        4.40246501, 4.45084375, 4.49922248, 4.54760122, 4.59597996,
        4.64435869, 4.69273743],
       [4.70483211, 4.75969015, 4.81454818, 4.86940621, 4.92426424,
        4.97912227, 5.0339803 , 5.08883833, 5.14369637, 5.1985544 ,
        5.25341243, 5.30827046],
       [5.2490929 , 5.31043023, 5.37176755, 5.43310488, 5.4944422 ,
        5.55577953, 5.61711686, 5.67845418, 5.73979151, 5.80112884,
        5.86246616, 5.92380349],
       [5.79335368, 5.8611703 , 5.92898693, 5.99680355, 6.06462017,
        6.13243679, 6.20025341, 6.26807003, 6.33588666, 6.40370328,
        6.4715199 , 6.53933652],
       [6.33761447, 6.41191038, 6.4862063 , 6.56050222, 6.63479813,
        6.70909405, 6.78338997, 6.85768588, 6.9319818 , 7.00627772,
        7.08057363, 7.15486955],
       [6.88187525, 6.96265046, 7.04342568, 7.12420089, 7.2049761 ,
        7.28575131, 7.36652652, 7.44730173, 7.52807695, 7.60885216,
        7.68962737, 7.77040258]]> : tensor<13x12xf32>) : tensor<13x12xf32>
  return
}