#ifndef __USB_HID_H__
#define __USB_HID_H__

#define HIDCLASS_HID          3
#define HIDSUBCLASS_NONE      0
#define HIDSUBCLASS_BOOT      1
#define HIDPROTOCOL_NONE      0
#define HIDPROTOCOL_KEYBOARD  1
#define HIDPROTOCOL_MOUSE     2

#define USAGEPAGE_GENERIC     0x01
#define USAGEPAGE_SIMULATOR   0x02
#define USAGEPAGE_KEYBOARD    0x07
#define USAGEPAGE_LEDS        0x08
#define USAGEPAGE_BUTTONS     0x09
#define USAGEPAGE_DIGITIZER   0x0D
//Generic Desktop
#define USAGE_POINTER         0x01 //CP
#define USAGE_MOUSE           0x02 //CA
#define USAGE_JOYSTICK        0x04 //CA
#define USAGE_GAMEPAD         0x05 //CA
#define USAGE_KEYBOARD        0x06 //CA
#define USAGE_KEYPAD          0x07 //CA
#define USAGE_MULTI_AXIS      0x08 //CA
#define USAGE_X               0x30 //DV
#define USAGE_Y               0x31 //DV
#define USAGE_Z               0x32 //DV
#define USAGE_RX              0x33 //DV, rotate X
#define USAGE_RY              0x34 //DV, rotate Y
#define USAGE_RZ              0x35 //DV, rotate Z
#define USAGE_SLIDER          0x36 //DV
#define USAGE_DIAL            0x37 //DV
#define USAGE_WHEEL           0x38 //DV
#define USAGE_HATSWITCH       0x39 //DV
//buttons
#define USAGE_BTN1            0x01
#define USAGE_BTN2            0x02
#define USAGE_BTN3            0x03
//digitizer
#define USAGE_DIGITIZER       0x01 //CA
#define USAGE_PEN             0x02 //CA
#define USAGE_TOUCHSCREEN     0x04 //CA
#define USAGE_TOUCHPAD        0x05 //CA
#define USAGE_3DDIGITIZER     0x08 //CA
#define USAGE_STYLUS          0x20 //CL
#define USAGE_PUCK            0x21 //CL
#define USAGE_FINGER          0x22 //CL
#define USAGE_TIP_PRESSURE    0x30 //DV
#define USAGE_IN_RANGE        0x32 //MC
#define USAGE_TOUCH           0x33 //MC
#define USAGE_UNTOUCH         0x34 //OSC
#define USAGE_TAP             0x35 //OSC
#define USAGE_DATAVALID       0x37 //MC
#define USAGE_TIPSWITCH       0x42 //MC
#define USAGE_TIPSWITCH2      0x43 //MC
#define USAGE_BARRELSWITCH    0x44 //MC
#define USAGE_ERASER          0x45 //MC

#define COLL_PHISICAL         0x00
#define COLL_APPLICATION      0x01
#define COLL_LOGICAL          0x02
#define COLL_REPORT           0x03
#define COLL_NAMED_ARRAY      0x04
#define COLL_USAGE_SWITCH     0x05
#define COLL_USAGE_MODIFIER   0x06

#define HID_DATA              (0<<0) //данный Item является переменной
#define HID_CONST             (1<<0) // -/- константой
#define HID_ARR               (0<<1) //представление как массив номеров активных элементов
#define HID_VAR               (1<<1) // -/- массив состояний
#define HID_ABS               (0<<2) //данные абсолютное
#define HID_REL               (1<<2) // -/- относительные
#define HID_NWRAP             (0<<3) //переполнения не будет
#define HID_WRAP              (1<<3) // -/- будет
#define HID_LIN               (0<<4) //даннве линейно зависят от "крутилки"
#define HID_NLIN              (1<<4) // -/- нелинейно (TODO)
#define HID_PREF              (0<<5) //данный Item имеет дефолтное состояние (например, кнопка)
#define HID_NOPREF            (1<<5) //-/- не имеет
#define HID_NONULL            (0<<6) //данный Item всегда выдает корректные значения
#define HID_NULL              (1<<6) // -/- может не выдавать значений (результат за пределами logical_min/max) если находится в дефолтном состоянии
#define HID_NOVOL             (0<<7) //значение output/feature не может меняться без ведома хоста
#define HID_VOL               (1<<7) //-/- может (поэтому лучше ему выставлять HID_REL)
#define HID_BITFLD            (0<<8)
#define HID_BUF               (1<<8)

#define USAGE_PAGE(page)        0x05, page
#define USAGE(usg)              0x09, usg
#define COLLECTION(coll, x...)  0xA1, coll, x 0xC0,
#define REPORT_ID(id)           0x85, id
#define INPUT_HID(val)                HID_VALUE_8(0x80, val)
#define INPUT_HID16(val)              HID_VALUE_16(0x80, val)
#define INPUT_HID24(val)              HID_VALUE_24(0x80, val)
#define OUTPUT_HID(val)               HID_VALUE_8(0x90, val)
#define OUTPUT_HID16(val)             HID_VALUE_16(0x90, val)
#define OUTPUT_HID24(val)             HID_VALUE_24(0x90, val)
#define FEATURE_HID(val)              HID_VALUE_8(0xB0, val)
#define FEATURE_HID16(val)            HID_VALUE_16(0xB0, val)
#define FEATURE_HID24(val)            HID_VALUE_24(0xB0, val)

#define LOGICAL_MINIMUM(size, val)    HID_VALUE_##size(0x14, val)
#define LOGICAL_MAXIMUM(size, val)    HID_VALUE_##size(0x24, val)
#define LOGICAL_MINMAX(min, max)        HID_VALUE_8(0x14, min), HID_VALUE_8(0x24, max)
#define LOGICAL_MINMAX16(min, max)      HID_VALUE_16(0x14, min), HID_VALUE_16(0x24, max)
#define LOGICAL_MINMAX24(min, max)      HID_VALUE_24(0x14, min), HID_VALUE_24(0x24, max)
#define PHISICAL_MINIMUM(size, val)   HID_VALUE_##size(0x34, val)
#define PHISICAL_MAXIMUM(size, val)   HID_VALUE_##size(0x44, val)
#define PHISICAL_MINMAX(min, max)       HID_VALUE_8(0x34, min), HID_VALUE_8(0x44, max)
#define PHISICAL_MINMAX16(min, max)     HID_VALUE_16(0x34, min), HID_VALUE_16(0x44, max)
#define PHISICAL_MINMAX24(min, max)     HID_VALUE_24(0x34, min), HID_VALUE_24(0x44, max)
#define UNIT_EXPONENT(size, val)      HID_VALUE_##size(0x54, val)
#define UNIT_VALUE(size, val)         HID_VALUE_##size(0x64, val)
#define REPORT_SIZE(size, val)        HID_VALUE_##size(0x74, val) //размер одного Item'а (в битах)
#define REPORT_COUNT(size, val)       HID_VALUE_##size(0x94, val) //количество Item'ов (в штуках)
#define REPORT_FMT(sz_bits, count)      HID_VALUE_8(0x74, sz_bits), HID_VALUE_8(0x94, count)
#define REPORT_FMT16(sz_bits, count)    HID_VALUE_16(0x74, sz_bits), HID_VALUE_16(0x94, count)
#define REPORT_FMT24(sz_bits, count)    HID_VALUE_24(0x74, sz_bits), HID_VALUE_24(0x94, count)

#define USAGE_MINIMUM(size, val)      HID_VALUE_##size(0x18, val)
#define USAGE_MAXIMUM(size, val)      HID_VALUE_##size(0x28, val)
#define USAGE_MINMAX(min, max)          HID_VALUE_8(0x18, min), HID_VALUE_8(0x28, max)
#define USAGE_MINMAX16(min, max)        HID_VALUE_16(0x18, min), HID_VALUE_16(0x28, max)
#define USAGE_MINMAX24(min, max)        HID_VALUE_24(0x18, min), HID_VALUE_24(0x28, max)

#define HID_VALUE_0(tag, x)  (tag | 0)
#define HID_VALUE_8(tag, x)  (tag | 1), x
#define HID_VALUE_16(tag, x) (tag | 2), (x & 0xFF), ((x>>8)&0xFF)
#define HID_VALUE_24(tag, x) (tag | 3), (x & 0xFF), ((x>>8)&0xFF), ((x>>16)&0xFF)

#define HID_KBDLED_NUMLOCK      0
#define HID_KBDLED_CAPSLOCK     1
#define HID_KBDLED_SCROLLLOCK   2
#define HID_KBDLED_COMPOSE      3 //модификатор Compose, используется для ввода некоторых символов, которых нет на обычной клавиатуре. Впрочем, похоже, немногих, так что смысла от нее нет
#define HID_KBDLED_KANA         4 //переключатель на ввод иероглифов (в т.ч. японских)?

#define HID_KBDMOD_LCTRL        0
#define HID_KBDMOD_LSHIFT       1
#define HID_KBDMOD_LALT         2
#define HID_KBDMOD_LGUI         3
#define HID_KBDMOD_RCTRL        4
#define HID_KBDMOD_RSHIFT       5
#define HID_KBDMOD_RALT         6
#define HID_KBDMOD_RGUI         7

/*
  CA - collection (application)
  CL - collection (logical)
  CP - collection (phisical)
  US - usage switch
  UM - usage midifier
  
  LC - Linear Control
  OOC - On/Off Control
  MC - Momentary Control (ABS, PREF). 1-событие наступило, 0-исчезло
  OSC - One Shot Control (REL, PREF). 0->1 - событие наступило, 1->0 когда-нибудь сбросить
  RTC - Re-Trigger Control (ABS, PREF) 

  SEL (ARR) - выбор из массива
  SV, Static Value (CONST, VAR, ABS) - A read-only multiple-bit value
  SF, Static Flag (CONST, VAR, ABS) - A read-only single-bit value
  DV, Dynamic Value (DATA, VAR, ABS) - A read/write multiple-bit value
  DF, Dynamic Flag (DATA, VAR, ABS) - A read/write single-bit value
*/

#endif
