//
// tables.c: opcode, register, directive, I/O port and constant tables for v32as,
// along with a case-insensitive hash lookup used by the lexer
//
// The opcode numbering, register aliases, I/O port numbers and port values come from
// the Vircon32 specification (CPU, GPU, SPU, controller chip documents) and agree with
// the official assembler's definitions.
//
////////////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <ctype.h>

#include "v32as.h"

////////////////////////////////////////////////////////////////////////////////////////
//
// The 64 opcodes: name, opcode number, operand class
//
#define  OPCODES(X)                                                                    \
    X(HLT,   0, NONE) X(WAIT,  1, NONE) X(JMP,   2, JMP)  X(CALL,  3, JMP)            \
    X(RET,   4, NONE) X(JT,    5, CJMP) X(JF,    6, CJMP) X(IEQ,   7, RI)             \
    X(INE,   8, RI)   X(IGT,   9, RI)   X(IGE,  10, RI)   X(ILT,  11, RI)             \
    X(ILE,  12, RI)   X(FEQ,  13, RF)   X(FNE,  14, RF)   X(FGT,  15, RF)             \
    X(FGE,  16, RF)   X(FLT,  17, RF)   X(FLE,  18, RF)   X(MOV,  19, MOV)            \
    X(LEA,  20, LEA)  X(PUSH, 21, R)    X(POP,  22, R)    X(IN,   23, IN)             \
    X(OUT,  24, OUT)  X(MOVS, 25, NONE) X(SETS, 26, NONE) X(CMPS, 27, R)             \
    X(CIF,  28, R)    X(CFI,  29, R)    X(CIB,  30, R)    X(CFB,  31, R)              \
    X(NOT,  32, R)    X(AND,  33, RI)   X(OR,   34, RI)   X(XOR,  35, RI)             \
    X(BNOT, 36, R)    X(SHL,  37, RI)   X(IADD, 38, RI)   X(ISUB, 39, RI)             \
    X(IMUL, 40, RI)   X(IDIV, 41, RI)   X(IMOD, 42, RI)   X(ISGN, 43, R)              \
    X(IMIN, 44, RI)   X(IMAX, 45, RI)   X(IABS, 46, R)    X(FADD, 47, RF)             \
    X(FSUB, 48, RF)   X(FMUL, 49, RF)   X(FDIV, 50, RF)   X(FMOD, 51, RF)             \
    X(FSGN, 52, R)    X(FMIN, 53, RF)   X(FMAX, 54, RF)   X(FABS, 55, R)              \
    X(FLR,  56, R)    X(CEIL, 57, R)    X(ROUND,58, R)    X(SIN,  59, R)              \
    X(ACOS, 60, R)    X(ATAN2,61, RR)   X(LOG,  62, R)    X(POW,  63, RR)

#define  OPNAME(n, v, c)   [v] = #n,
static const char *const opcode_names[64] = { OPCODES(OPNAME) };

typedef struct { const char *name; unsigned value; } named_t;

////////////////////////////////////////////////////////////////////////////////////////
//
// I/O ports (also exposed as constants) and the values accepted by the command ports
//
static const named_t  io_ports[] =
{
    { "TIM_CurrentDate", 0x000 },
    { "TIM_CurrentTime", 0x001 },
    { "TIM_FrameCounter", 0x002 },
    { "TIM_CycleCounter", 0x003 },
    { "RNG_CurrentValue", 0x100 },
    { "GPU_Command", 0x200 },
    { "GPU_RemainingPixels", 0x201 },
    { "GPU_ClearColor", 0x202 },
    { "GPU_MultiplyColor", 0x203 },
    { "GPU_ActiveBlending", 0x204 },
    { "GPU_SelectedTexture", 0x205 },
    { "GPU_SelectedRegion", 0x206 },
    { "GPU_DrawingPointX", 0x207 },
    { "GPU_DrawingPointY", 0x208 },
    { "GPU_DrawingScaleX", 0x209 },
    { "GPU_DrawingScaleY", 0x20A },
    { "GPU_DrawingAngle", 0x20B },
    { "GPU_RegionMinX", 0x20C },
    { "GPU_RegionMinY", 0x20D },
    { "GPU_RegionMaxX", 0x20E },
    { "GPU_RegionMaxY", 0x20F },
    { "GPU_RegionHotspotX", 0x210 },
    { "GPU_RegionHotspotY", 0x211 },
    { "SPU_Command", 0x300 },
    { "SPU_GlobalVolume", 0x301 },
    { "SPU_SelectedSound", 0x302 },
    { "SPU_SelectedChannel", 0x303 },
    { "SPU_SoundLength", 0x304 },
    { "SPU_SoundPlayWithLoop", 0x305 },
    { "SPU_SoundLoopStart", 0x306 },
    { "SPU_SoundLoopEnd", 0x307 },
    { "SPU_ChannelState", 0x308 },
    { "SPU_ChannelAssignedSound", 0x309 },
    { "SPU_ChannelVolume", 0x30A },
    { "SPU_ChannelSpeed", 0x30B },
    { "SPU_ChannelLoopEnabled", 0x30C },
    { "SPU_ChannelPosition", 0x30D },
    { "INP_SelectedGamepad", 0x400 },
    { "INP_GamepadConnected", 0x401 },
    { "INP_GamepadLeft", 0x402 },
    { "INP_GamepadRight", 0x403 },
    { "INP_GamepadUp", 0x404 },
    { "INP_GamepadDown", 0x405 },
    { "INP_GamepadButtonStart", 0x406 },
    { "INP_GamepadButtonA", 0x407 },
    { "INP_GamepadButtonB", 0x408 },
    { "INP_GamepadButtonX", 0x409 },
    { "INP_GamepadButtonY", 0x40A },
    { "INP_GamepadButtonL", 0x40B },
    { "INP_GamepadButtonR", 0x40C },
    { "CAR_Connected", 0x500 },
    { "CAR_ProgramROMSize", 0x501 },
    { "CAR_NumberOfTextures", 0x502 },
    { "CAR_NumberOfSounds", 0x503 },
    { "MEM_Connected", 0x600 },
};

static const named_t  io_port_values[] =
{
    { "GPUCommand_ClearScreen", 0x10 },
    { "GPUCommand_DrawRegion", 0x11 },
    { "GPUCommand_DrawRegionZoomed", 0x12 },
    { "GPUCommand_DrawRegionRotated", 0x13 },
    { "GPUCommand_DrawRegionRotozoomed", 0x14 },
    { "GPUBlendingMode_Alpha", 0x20 },
    { "GPUBlendingMode_Add", 0x21 },
    { "GPUBlendingMode_Subtract", 0x22 },
    { "SPUCommand_PlaySelectedChannel", 0x30 },
    { "SPUCommand_PauseSelectedChannel", 0x31 },
    { "SPUCommand_StopSelectedChannel", 0x32 },
    { "SPUCommand_PauseAllChannels", 0x33 },
    { "SPUCommand_ResumeAllChannels", 0x34 },
    { "SPUCommand_StopAllChannels", 0x35 },
    { "SPUChannelState_Stopped", 0x40 },
    { "SPUChannelState_Paused", 0x41 },
    { "SPUChannelState_Playing", 0x42 },
};

////////////////////////////////////////////////////////////////////////////////////////
//
// Keyword hash table (open addressing, case-insensitive FNV-1a)
//
#define  KW_TABLE_SIZE   512          // power of two; the table holds ~165 entries

static kw_t           kw_store[KW_TABLE_SIZE];
static unsigned char  kw_used [KW_TABLE_SIZE];
static unsigned char  port_known[0x700];
static int            kw_ready  = 0;

static uint32_t  kw_hash (const char *s, size_t n)
{
    uint32_t  h  = 2166136261u;
    size_t    i;

    for (i = 0; i < n; i++)
    {
        h ^= (uint32_t) toupper ((unsigned char) s[i]);
        h *= 16777619u;
    }

    return (h);
}

static int  kw_equal (const char *a, const char *b, size_t n)
{
    size_t  i;

    if (strlen (b) != n)
    {
        return (0);
    }

    for (i = 0; i < n; i++)
    {
        if (toupper ((unsigned char) a[i]) != toupper ((unsigned char) b[i]))
        {
            return (0);
        }
    }

    return (1);
}

static void  kw_add (const char *name, kw_kind_t kind, int cls, uint32_t value)
{
    size_t  n    = strlen (name);
    size_t  slot = kw_hash (name, n) & (KW_TABLE_SIZE - 1);

    while (kw_used[slot])
    {
        slot = (slot + 1) & (KW_TABLE_SIZE - 1);
    }

    kw_used[slot]          = 1;
    kw_store[slot].name    = name;
    kw_store[slot].kind    = kind;
    kw_store[slot].cls     = cls;
    kw_store[slot].value   = value;
}

static void  kw_init (void)
{
    static char  regnames[16][4];
    size_t       i;

    #define  ADD_OPCODE(n, v, c)   kw_add (#n, KW_OPCODE, OC_##c, v);
    OPCODES (ADD_OPCODE)

    kw_add ("integer",  KW_DIRECTIVE, DIR_INTEGER,  0);
    kw_add ("float",    KW_DIRECTIVE, DIR_FLOAT,    0);
    kw_add ("string",   KW_DIRECTIVE, DIR_STRING,   0);
    kw_add ("zstring",  KW_DIRECTIVE, DIR_ZSTRING,  0);
    kw_add ("pointer",  KW_DIRECTIVE, DIR_POINTER,  0);
    kw_add ("datafile", KW_DIRECTIVE, DIR_DATAFILE, 0);

    for (i = 0; i < 11; i++)
    {
        sprintf (regnames[i], "R%d", (int) i);
        kw_add (regnames[i], KW_REGISTER, 0, (uint32_t) i);
    }

    kw_add ("CR", KW_REGISTER, 0, 11);
    kw_add ("SR", KW_REGISTER, 0, 12);
    kw_add ("DR", KW_REGISTER, 0, 13);
    kw_add ("BP", KW_REGISTER, 0, 14);
    kw_add ("SP", KW_REGISTER, 0, 15);

    // R11..R15 are the same registers as CR, SR, DR, BP and SP
    {
        static const char *const alias[] = { "R11", "R12", "R13", "R14", "R15" };

        for (i = 0; i < 5; i++)
        {
            kw_add (alias[i], KW_REGISTER, 0, (uint32_t) (11 + i));
        }
    }

    for (i = 0; i < sizeof (io_ports) / sizeof (io_ports[0]); i++)
    {
        kw_add (io_ports[i].name, KW_PORT, 0, io_ports[i].value);
        port_known[io_ports[i].value] = 1;
    }

    for (i = 0; i < sizeof (io_port_values) / sizeof (io_port_values[0]); i++)
    {
        kw_add (io_port_values[i].name, KW_PORTVALUE, 0, io_port_values[i].value);
    }

    kw_add ("IP", KW_SYSREG, 0, SYS_IP);
    kw_add ("IR", KW_SYSREG, 0, SYS_IR);
    kw_add ("IV", KW_SYSREG, 0, SYS_IV);
    kw_add ("true",  KW_CONSTANT, 0, 1);
    kw_add ("false", KW_CONSTANT, 0, 0);

    kw_ready = 1;
}

const kw_t  *kw_lookup (const char *s, size_t n)
{
    size_t  slot;

    if (!kw_ready)
    {
        kw_init ();
    }

    slot = kw_hash (s, n) & (KW_TABLE_SIZE - 1);

    while (kw_used[slot])
    {
        if (kw_equal (s, kw_store[slot].name, n))
        {
            return (&kw_store[slot]);
        }

        slot = (slot + 1) & (KW_TABLE_SIZE - 1);
    }

    return (NULL);
}

const char  *opcode_name (unsigned opcode)
{
    return ((opcode < 64) ? opcode_names[opcode] : "???");
}

int  port_is_known (unsigned port)
{
    if (!kw_ready)
    {
        kw_init ();
    }

    return ((port < sizeof (port_known)) ? port_known[port] : 0);
}
