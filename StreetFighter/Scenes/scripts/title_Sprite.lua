local Script = {}

-- IntroFrames
Script.IntroFrames = {
    [1] = { x = 4, y = 6, width = 384, height = 224 },
    [2] = { x = 390, y = 6, width = 384, height = 224 },
    [3] = { x = 776, y = 6, width = 384, height = 224 },
    [4] = { x = 4, y = 233, width = 384, height = 224 },
    [5] = { x = 390, y = 233, width = 384, height = 224 },
    [6] = { x = 776, y = 233, width = 384, height = 224 },
    [7] = { x = 4, y = 461, width = 384, height = 224 },
    [8] = { x = 446, y = 472, width = 78, height = 30 },
    [9] = { x = 530, y = 472, width = 56, height = 60 },
    [10] = { x = 635, y = 492, width = 48, height = 49 },
    [11] = { x = 662, y = 492, width = 60, height = 51 },
    [12] = { x = 753, y = 471, width = 75, height = 81 },
    [13] = { x = 823, y = 484, width = 25, height = 35 },
    [14] = { x = 843, y = 478, width = 48, height = 36 },
    [15] = { x = 904, y = 473, width = 26, height = 31 },
    [16] = { x = 555, y = 499, width = 76, height = 45 },
    [17] = { x = 607, y = 507, width = 44, height = 34 },
    [18] = { x = 730, y = 460, width = 300, height = 120 },
    [19] = { x = 841, y = 524, width = 22, height = 40 },
    [20] = { x = 856, y = 524, width = 32, height = 39 },
    [21] = { x = 885, y = 517, width = 29, height = 41 },
    [22] = { x = 892, y = 498, width = 65, height = 65 },
    [23] = { x = 917, y = 512, width = 28, height = 31 },
    [24] = { x = 928, y = 498, width = 81, height = 68 },
    [25] = { x = 434, y = 593, width = 122, height = 80 },
    [26] = { x = 582, y = 599, width = 104, height = 79 },
    [27] = { x = 702, y = 601, width = 88, height = 76 },
    [28] = { x = 816, y = 603, width = 112, height = 72 },
}

-- RyuFrames (80 elementos)
Script.RyuFrames = {
    [1] = { x = 8, y = 9, width = 38, height = 95 },
    [2] = { x = 56, y = 10, width = 45, height = 94 },
    [3] = { x = 112, y = 11, width = 45, height = 93 },
    [4] = { x = 168, y = 9, width = 35, height = 95 },
    [5] = { x = 216, y = 10, width = 45, height = 94 },
    [6] = { x = 272, y = 11, width = 47, height = 93 },
    [7] = { x = 328, y = 12, width = 46, height = 92 },
    [8] = { x = 384, y = 14, width = 63, height = 90 },
    [9] = { x = 456, y = 13, width = 55, height = 91 },
    [10] = { x = 8, y = 121, width = 45, height = 87 },
    [11] = { x = 64, y = 113, width = 32, height = 95 },
    [12] = { x = 368, y = 129, width = 75, height = 79 },
    [13] = { x = 104, y = 144, width = 36, height = 64 },
    [14] = { x = 153, y = 160, width = 47, height = 48 },
    [15] = { x = 208, y = 161, width = 48, height = 47 },
    [16] = { x = 264, y = 161, width = 48, height = 47 },
    [17] = { x = 320, y = 144, width = 38, height = 64 },
    [18] = { x = 456, y = 144, width = 45, height = 64 },
    [19] = { x = 512, y = 144, width = 47, height = 64 },
    [20] = { x = 8, y = 216, width = 45, height = 80 },
    [21] = { x = 64, y = 233, width = 47, height = 63 },
    [22] = { x = 120, y = 232, width = 48, height = 64 },
    [23] = { x = 176, y = 236, width = 48, height = 60 },
    [24] = { x = 232, y = 234, width = 48, height = 62 },
    [25] = { x = 288, y = 236, width = 48, height = 60 },
    [26] = { x = 344, y = 234, width = 53, height = 62 },
    [27] = { x = 408, y = 237, width = 80, height = 59 },
    [28] = { x = 8, y = 308, width = 61, height = 92 },
    [29] = { x = 80, y = 308, width = 75, height = 92 },
    [30] = { x = 168, y = 311, width = 95, height = 89 },
    [31] = { x = 272, y = 308, width = 55, height = 92 },
    [32] = { x = 336, y = 308, width = 47, height = 92 },
    [33] = { x = 392, y = 307, width = 56, height = 93 },
    [34] = { x = 456, y = 307, width = 48, height = 93 },
    [35] = { x = 8, y = 420, width = 89, height = 84 },
    [36] = { x = 104, y = 421, width = 87, height = 83 },
    [37] = { x = 200, y = 409, width = 54, height = 95 },
    [38] = { x = 264, y = 411, width = 47, height = 93 },
    [39] = { x = 320, y = 416, width = 59, height = 88 },
    [40] = { x = 392, y = 414, width = 40, height = 90 },
    [41] = { x = 440, y = 422, width = 56, height = 82 },
    [42] = { x = 8, y = 515, width = 85, height = 61 },
    [43] = { x = 104, y = 515, width = 36, height = 61 },
    [44] = { x = 152, y = 520, width = 88, height = 56 },
    [45] = { x = 248, y = 512, width = 78, height = 64 },
    [46] = { x = 336, y = 513, width = 74, height = 63 },
    [47] = { x = 424, y = 513, width = 69, height = 63 },
    [48] = { x = 8, y = 613, width = 63, height = 91 },
    [49] = { x = 80, y = 608, width = 24, height = 96 },
    [50] = { x = 112, y = 612, width = 48, height = 92 },
    [51] = { x = 168, y = 615, width = 85, height = 89 },
    [52] = { x = 264, y = 615, width = 64, height = 89 },
    [53] = { x = 337, y = 615, width = 61, height = 89 },
    [54] = { x = 408, y = 586, width = 49, height = 118 },
    [55] = { x = 464, y = 588, width = 39, height = 116 },
    [56] = { x = 8, y = 720, width = 62, height = 80 },
    [57] = { x = 208, y = 712, width = 55, height = 88 },
    [58] = { x = 80, y = 738, width = 52, height = 62 },
    [59] = { x = 144, y = 744, width = 55, height = 56 },
    [60] = { x = 272, y = 738, width = 48, height = 62 },
    [61] = { x = 328, y = 741, width = 64, height = 59 },
    [62] = { x = 96, y = 824, width = 86, height = 80 },
    [63] = { x = 192, y = 816, width = 56, height = 88 },
    [64] = { x = 256, y = 817, width = 64, height = 87 },
    [65] = { x = 328, y = 809, width = 52, height = 95 },
    [66] = { x = 392, y = 809, width = 54, height = 95 },
    [67] = { x = 456, y = 810, width = 52, height = 94 },
    [68] = { x = 8, y = 825, width = 77, height = 79 },
    [69] = { x = 64, y = 912, width = 46, height = 112 },
    [70] = { x = 8, y = 932, width = 46, height = 92 },
    [71] = { x = 120, y = 961, width = 63, height = 63 },
    [72] = { x = 320, y = 946, width = 227, height = 35 },
    [73] = { x = 192, y = 995, width = 103, height = 29 },
    [74] = { x = 320, y = 989, width = 128, height = 8 },
    [75] = { x = 8, y = 1032, width = 94, height = 160 },
    [76] = { x = 112, y = 1034, width = 94, height = 158 },
    [77] = { x = 424, y = 1048, width = 101, height = 144 },
    [78] = { x = 536, y = 1064, width = 94, height = 128 },
    [79] = { x = 216, y = 1066, width = 94, height = 126 },
    [80] = { x = 320, y = 1067, width = 94, height = 125 },
}

-- SagatFrames
Script.SagatFrames = {
    [1] = { x = 8, y = 17, width = 64, height = 119 },
    [2] = { x = 80, y = 17, width = 80, height = 119 },
    [3] = { x = 168, y = 17, width = 76, height = 119 },
    [4] = { x = 328, y = 9, width = 68, height = 127 },
    [5] = { x = 256, y = 25, width = 64, height = 111 },
    [6] = { x = 428, y = 89, width = 299, height = 26 },
    [7] = { x = 428, y = 123, width = 128, height = 8 },
    [8] = { x = 8, y = 153, width = 72, height = 119 },
    [9] = { x = 88, y = 153, width = 83, height = 119 },
    [10] = { x = 184, y = 157, width = 87, height = 115 },
    [11] = { x = 280, y = 145, width = 56, height = 127 },
    [12] = { x = 344, y = 145, width = 64, height = 127 },
    [13] = { x = 416, y = 145, width = 56, height = 127 },
    [14] = { x = 480, y = 153, width = 96, height = 119 },
    [15] = { x = 584, y = 153, width = 75, height = 119 },
    [16] = { x = 672, y = 161, width = 119, height = 111 },
    [17] = { x = 8, y = 289, width = 71, height = 119 },
    [18] = { x = 88, y = 289, width = 59, height = 119 },
    [19] = { x = 160, y = 291, width = 63, height = 117 },
    [20] = { x = 368, y = 281, width = 60, height = 127 },
    [21] = { x = 440, y = 280, width = 62, height = 128 },
    [22] = { x = 233, y = 376, width = 127, height = 32 },
}

-- AdonStageFrames
Script.AdonStageFrames = {
    [1] = { x = 8, y = 16, width = 768, height = 224 },
    [2] = { x = 8, y = 256, width = 896, height = 176 },
}

-- BirdieStageFrames
Script.BirdieStageFrames = {
    [1] = { x = 8, y = 16, width = 768, height = 224 },
    [2] = { x = 8, y = 256, width = 896, height = 144 },
}

-- EagleStageFrames
Script.EagleStageFrames = {
    [1] = { x = 8, y = 26, width = 768, height = 214 },
    [2] = { x = 8, y = 256, width = 896, height = 176 },
}

-- FontFrames
Script.FontFrames = {
    [1] = { x = 16, y = 8, width = 64, height = 96 },
    [2] = { x = 88, y = 8, width = 64, height = 96 },
    [3] = { x = 160, y = 8, width = 64, height = 96 },
    [4] = { x = 232, y = 8, width = 64, height = 96 },
    [5] = { x = 320, y = 16, width = 104, height = 40 },
    [6] = { x = 344, y = 72, width = 64, height = 128 },
    [7] = { x = 16, y = 112, width = 64, height = 96 },
    [8] = { x = 88, y = 112, width = 64, height = 96 },
    [9] = { x = 160, y = 112, width = 64, height = 96 },
    [10] = { x = 232, y = 112, width = 64, height = 96 },
    [11] = { x = 16, y = 216, width = 96, height = 112 },
    [12] = { x = 120, y = 216, width = 96, height = 112 },
    [13] = { x = 224, y = 216, width = 96, height = 112 },
    [14] = { x = 328, y = 216, width = 96, height = 112 },
    [15] = { x = 16, y = 336, width = 96, height = 112 },
    [16] = { x = 120, y = 336, width = 96, height = 112 },
    [17] = { x = 224, y = 336, width = 96, height = 112 },
    [18] = { x = 328, y = 336, width = 96, height = 112 },
}

function Script:onCreate(entity)
    self.entity = entity
    self.currentFrame = 1
    self.timer = 0
    self.frameDuration = 0.15

    log.info(string.format("[%s] Script with Ryu (%d), Sagat (%d), Intro (%d), Fonts (%d)",
        tostring(entity),
        #self.RyuFrames,
        #self.SagatFrames,
        #self.IntroFrames,
        #self.FontFrames))
end

function Script:onUpdate(entity, dt)
    self.timer = self.timer + dt
    if self.timer >= self.frameDuration then
        self.timer = 0

        self.currentFrame = (self.currentFrame % #self.IntroFrames) + 1

        local frame = self.IntroFrames[18]


        Component.setSpriteRect(entity, frame.x, frame.y, frame.width, frame.height)
    end
end

function Script:onDestroy(entity)
    log.info("[" .. tostring(entity) .. "] Script destroyed")
end

return Script
