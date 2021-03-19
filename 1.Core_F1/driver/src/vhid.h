#ifndef __MYHID_H__
#define __MYHID_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <wchar.h>
  #include <inttypes.h>

//реальный тип зависит от реализации, так что использовать только функции из этой библиотеки
typedef void hiddevice_t;

/*   Поиск и инициализация устройства по заданным параметрам. Если хотя бы один параметр не задан
 * то идет просто вывод на stdout обнаруженных устройств.
 *   Входные параметры:
 * vid, pid - vendor ID, product ID устройства
 * man[] - Manufacturer, производитель. В формате wchar_t, то есть константу можно передать как L"Manufacturer" например
 * prod[] - Product, название устройства. Аналогично man[]
 *   Выходное значение:
 * hiddevice_t, указатель на область памяти, выделенный данной функцией. В случае ошибки - NULL.
 * Это значение надо ОБЯЗАТЕЛЬНО сохранить и в конце освободить в функции HidCloseDevice()
 */
hiddevice_t* HidOpen(uint16_t vid, uint16_t pid, const wchar_t *man, const wchar_t *prod);
/*  Закрытие открытого ранее устройства */
void HidClose(hiddevice_t *dev);
/*  Проверка удачно ли открылось устройство (bool) */
char HidIsConnected(hiddevice_t *dev);
/*  Отобразить все устройства  */
void HidDisplay();
/*  Передача массива данных в открытое устройство */
char HidWrite(hiddevice_t *dev, void *buf, size_t size);
/*  Прием массива данных заданного размера из устройства */
char HidRead(hiddevice_t *dev, void *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif
