//
// Created by BaiYang on 2023-04-12.
//

#ifndef MUSICPLAYER_ANDPLAYSTATUS_H
#define MUSICPLAYER_ANDPLAYSTATUS_H


class AndPlayStatus {
public:
    bool exit;
    bool play  = false;
    bool puase = false;
    bool seek  = false;

public:
    AndPlayStatus();
};


#endif //MUSICPLAYER_ANDPLAYSTATUS_H
