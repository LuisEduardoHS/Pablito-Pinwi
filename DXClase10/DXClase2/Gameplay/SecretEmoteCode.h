#ifndef GC_SECRET_EMOTE_CODE_H
#define GC_SECRET_EMOTE_CODE_H

// Physical key-down edges only: six 6s, then seven 7s, with no other key.
// No timeout: focus loss, menus and restart explicitly clear partial input.
class SecretEmoteCode
{
    unsigned progress=0;
public:
    void Reset(){progress=0;}
    bool Press(unsigned key,bool repeat=false)
    {
        if(repeat)return false;
        if(key==0x66)key='6'; // VK_NUMPAD6/7; no Windows dependency in this matcher.
        if(key==0x67)key='7';
        const unsigned expected=progress<6?'6':'7';
        if(key!=expected){progress=key=='6'?1:0;return false;}
        if(++progress==13){Reset();return true;}
        return false;
    }
};
#endif
