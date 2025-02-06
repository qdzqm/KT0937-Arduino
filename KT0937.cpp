#include "KT0937_D8.h"
#include <Wire.h>
//#include <cstdio>

// I2C書き込み関数
KT0937_D8_Error kt0937_writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(KT0937_D8_ADDR);
  Wire.write(reg);
  Wire.write(value);
  if (Wire.endTransmission() != 0) {
    return KT0937_D8_Error::ERROR_I2C;
  }
  return KT0937_D8_Error::OK;
}

// I2C読み取り関数
KT0937_D8_Error kt0937_readRegister(uint8_t reg, uint8_t *value) {
  Wire.beginTransmission(KT0937_D8_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return KT0937_D8_Error::ERROR_I2C;
  }
  if (Wire.requestFrom(KT0937_D8_ADDR, 1) != 1) {
    return KT0937_D8_Error::ERROR_I2C;
  }
  *value = Wire.read();
  return KT0937_D8_Error::OK;
}

// 初期化
// KT0937_D8_Error kt0937_init() {
//   Wire.begin();
//   Wire.setClock(100000);  // 100kHz I2C clock

//   uint8_t device_id;
//   if (kt0937_readRegister(REG_DEVICEID0, &device_id) != KT0937_D8_Error::OK) {
//     return KT0937_D8_Error::ERROR_I2C;
//   }
//   if (device_id != 0x82) {  // デバイスIDの確認（実際の値はデータシートを参照）
//     return KT0937_D8_Error::ERROR_DEVICE;
//   }

//   // 初期設定
//   kt0937_writeRegister(REG_RXCFG0, 0x10);  // DSPリセット
//   delay(100);  // リセット待ち

//   return KT0937_D8_Error::OK;
// }
KT0937_D8_Error kt0937_init() {
  Wire.begin();
  Wire.setClock(100000);  // 100kHz I2C clock

  kt0937_writeRegister(0x4E, 0x32); //Setting DEPOP_TC<2:0> and AUDV_DCLVL<2:0> register to 0x32(0011,0010) 
  kt0937_writeRegister(0x04, 0x00); //Setting DIVIDERP<10:8> to 0
  kt0937_writeRegister(0x05, 0x01); //Setting DIVIDERP<7:0> to 0x01
  kt0937_writeRegister(0x06, 0x02); //Setting DIVIDERN<10:8> to 0x02(0000,0010)
  kt0937_writeRegister(0x07, 0x9C); //Setting DIVIDERN<7:0> to 0x9C(datasheet recommended)
  kt0937_writeRegister(0x08, 0x08); 
  kt0937_writeRegister(0x09, 0x00);
  kt0937_writeRegister(0x0A, 0x00); //Setting FPFD<19:0>
  kt0937_writeRegister(0x0D, 0xC3); //Setting RCLK_EN
  kt0937_writeRegister(0x04, 0x80); //Setting SYSCLK_CFGOK to 1(1000,0000)
  uint8_t poweron_finish=0x80; //0x80 means the value of reg 0x1B is (1000,0000)
  while (poweron_finish == 0x80){
    kt0937_readRegister(0x1B, &poweron_finish);
    delay(10);
  } //Wait until the POWERON_FINISH flag was 1, which means the value of reg 0x1B is 0x84(1000,0100);
  kt0937_writeRegister(0x62, 0x41); //Setting FLT_SEL<2:0> register to 0x41(0100,0001)


  return KT0937_D8_Error::OK;
}
// モード設定
KT0937_D8_Error kt0937_setMode(KT0937_D8_Mode mode) {
  uint8_t value;
  if (kt0937_readRegister(0x88, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  switch (mode) {
    case KT0937_D8_Mode::FM:
      value &= ~0x40;  // AM_FMビットをクリア
      break;
    case KT0937_D8_Mode::MW:
    case KT0937_D8_Mode::SW:
      value |= 0x40;  // AM_FMビットをセット
      break;
    default:
      return KT0937_D8_Error::ERROR_PARAM;
  }

  return kt0937_writeRegister(REG_FMCHAN0, value);
}

// 周波数読み取り関数（新規追加）
KT0937_D8_Error kt0937_getFrequency(uint32_t *frequency) {
  uint8_t mode, high_byte, low_byte;
  
  if (kt0937_readRegister(0x88, &mode) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  high_byte = mode & 0x0F;

  if (kt0937_readRegister(REG_FMCHAN1, &low_byte) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  uint16_t channel = (high_byte << 8) | low_byte;

  if (!(mode & 0x40)) {  // FMモード
    *frequency = channel * 50 + 87500;  // 76MHz基準、50kHzステップ
  } else {  // AMモード
    *frequency = channel + 531;  // 522kHz基準
  }

  return KT0937_D8_Error::OK;
}

// 周波数設定
KT0937_D8_Error kt0937_setFrequency(uint32_t frequency) {
  uint8_t mode;
  if (kt0937_readRegister(0x88, &mode) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  uint16_t channel;
  if (!(mode & 0x40)) {  // FMモード
    channel = (frequency - 87500) / 50;  // 76MHz基準、50kHzステップ
  } else {  // AMモード
    channel = frequency - 531;  // 522kHz基準
  }

  uint8_t high_byte = (channel >> 8) & 0x0F;
  uint8_t low_byte = channel & 0xFF;

  if (kt0937_writeRegister(REG_FMCHAN0, (mode & 0xF0) | high_byte) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  return kt0937_writeRegister(REG_FMCHAN1, low_byte);
}

// ボリューム設定
KT0937_D8_Error kt0937_setVolume(uint8_t volume) {
  if (volume > 31) return KT0937_D8_Error::ERROR_PARAM;
  return kt0937_writeRegister(REG_RXCFG1, volume);
}

// ミュート設定
KT0937_D8_Error kt0937_setMute(bool mute) {
  uint8_t value;
  if (kt0937_readRegister(REG_RXCFG1, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  if (mute) {
    value &= ~0x1F;  // ボリュームビットをクリア
  } else {
    value |= 0x1F;  // ボリュームビットを最大に設定
  }

  return kt0937_writeRegister(REG_RXCFG1, value);
}

// RSSI取得
/*KT0937_D8_Error kt0937_getRSSI(uint8_t *rssi) {
  return kt0937_readRegister(REG_STATUS8, rssi);
}*/
KT0937_D8_Error kt0937_getRSSI(int8_t *rssi) {
  uint8_t raw_rssi;
  KT0937_D8_Error err = kt0937_readRegister(REG_STATUS8, &raw_rssi);
  
  if (err == KT0937_D8_Error::OK) {
    // KT0937-D8のデータシートに基づいてRSSI値を計算
    *rssi = -110 + (int8_t)(raw_rssi & 0x7F);
  }
  
  return err;
}

// SNR取得
KT0937_D8_Error kt0937_getSNR(uint8_t *snr) {
  return kt0937_readRegister(REG_STATUS4, snr);
}

// ステレオ確認
KT0937_D8_Error kt0937_isStereo(bool *is_stereo) {
  uint8_t value;
  KT0937_D8_Error err = kt0937_readRegister(REG_STATUS0, &value);
  if (err == KT0937_D8_Error::OK) {
    *is_stereo = (value & 0x01) ? true : false;
  }
  return err;
}

// AFC設定
KT0937_D8_Error kt0937_setAFC(bool enable) {
  uint8_t value;
  if (kt0937_readRegister(REG_AFC2, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  if (enable) {
    value &= ~0x40;  // AFCDビットをクリア
  } else {
    value |= 0x40;  // AFCDビットをセット
  }

  return kt0937_writeRegister(REG_AFC2, value);
}

// ソフトミュート設定
KT0937_D8_Error kt0937_setSoftMute(bool enable) {
  uint8_t value;
  if (kt0937_readRegister(REG_MUTECFG0, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  if (enable) {
    value &= ~0x80;  // FM_DSMUTEビットをクリア
    value &= ~0x40;  // MW_DSMUTEビットをクリア
  } else {
    value |= 0x80;  // FM_DSMUTEビットをセット
    value |= 0x40;  // MW_DSMUTEビットをセット
  }

  return kt0937_writeRegister(REG_MUTECFG0, value);
}

// バス設定
KT0937_D8_Error kt0937_setBass(uint8_t bass_boost) {
  if (bass_boost > 3) return KT0937_D8_Error::ERROR_PARAM;

  uint8_t value;
  if (kt0937_readRegister(REG_SOUNDCFG, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  value = (value & 0xCF) | (bass_boost << 4);
  return kt0937_writeRegister(REG_SOUNDCFG, value);
}

// FM空間設定
KT0937_D8_Error kt0937_setFMSpace(KT0937_D8_FMSpace space) {
  uint8_t value;
  if (kt0937_readRegister(REG_BANDCFG2, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  value = (value & 0x3F) | ((uint8_t)space << 6);
  return kt0937_writeRegister(REG_BANDCFG2, value);
}

// AM空間設定
KT0937_D8_Error kt0937_setAMSpace(KT0937_D8_AMSpace space) {
  uint8_t value;
  if (kt0937_readRegister(REG_BANDCFG2, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  value = (value & 0xFC) | (uint8_t)space;
  return kt0937_writeRegister(REG_BANDCFG2, value);
}

// SW空間設定
KT0937_D8_Error kt0937_setSWSpace(KT0937_D8_SWSpace space) {
  uint8_t value;
  if (kt0937_readRegister(REG_BANDCFG3, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  value = (value & 0xFC) | (uint8_t)space;
  return kt0937_writeRegister(REG_BANDCFG3, value);
}

// チャンネル有効性確認
KT0937_D8_Error kt0937_isChannelValid(bool *is_valid) {
  uint8_t value;
  KT0937_D8_Error err = kt0937_readRegister(REG_STATUS0, &value);
  if (err == KT0937_D8_Error::OK) {
    *is_valid = (value & 0x04) ? true : false;
  }
  return err;
}

// FMステレオブレンド設定
KT0937_D8_Error kt0937_setFMStereoBlend(uint8_t start_level, uint8_t stop_level) {
  return kt0937_writeRegister(REG_DSPCFG2, (start_level << 4) | stop_level);
}

// AMチャンネルフィルタ設定
KT0937_D8_Error kt0937_setAMChannelFilter(uint8_t bandwidth) {
  if (bandwidth > 4) return KT0937_D8_Error::ERROR_PARAM;

  uint8_t value;
  if (kt0937_readRegister(REG_AMDSP0, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  value = (value & 0xF8) | bandwidth;
  return kt0937_writeRegister(REG_AMDSP0, value);
}

// スタンバイモード設定
// スタンバイモード設定（続き）
KT0937_D8_Error kt0937_setStandby(bool enable) {
  uint8_t value;
  if (kt0937_readRegister(REG_RXCFG0, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  if (enable) {
    value |= 0x20;  // STDBYビットをセット
  } else {
    value &= ~0x20;  // STDBYビットをクリア
  }

  return kt0937_writeRegister(REG_RXCFG0, value);
}

// 割り込み設定
KT0937_D8_Error kt0937_configureInterrupt(bool enable, bool active_high, bool edge_triggered) {
  uint8_t value = 0;

  if (enable) {
    value |= 0x80;  // TUNE_INT_ENビットをセット
  }
  if (active_high) {
    value |= 0x40;  // TUNE_INT_PLビットをセット
  }
  if (edge_triggered) {
    value |= 0x20;  // TUNE_INT_MODEビットをセット
  }

  return kt0937_writeRegister(REG_SOFTMUTE5, value);
}

// キーモード設定
KT0937_D8_Error kt0937_setKeyMode() {
  uint8_t value;
  if (kt0937_readRegister(REG_GPIOCFG2, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  value = (value & 0xFC) | 0x01;  // CH_PIN<1:0>を01に設定
  return kt0937_writeRegister(REG_GPIOCFG2, value);
}

// ダイヤルモード設定
KT0937_D8_Error kt0937_setDialMode() {
  uint8_t value;
  if (kt0937_readRegister(REG_GPIOCFG2, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  value = (value & 0xFC) | 0x02;  // CH_PIN<1:0>を10に設定
  return kt0937_writeRegister(REG_GPIOCFG2, value);
}

// オーディオゲイン設定
KT0937_D8_Error kt0937_setAudioGain(uint8_t gain) {
  if (gain > 7) return KT0937_D8_Error::ERROR_PARAM;

  uint8_t value;
  if (kt0937_readRegister(REG_DSPCFG0, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  value = (value & 0x8F) | (gain << 4);
  return kt0937_writeRegister(REG_DSPCFG0, value);
}

// クリスタル周波数設定
KT0937_D8_Error kt0937_setCrystalFrequency(uint32_t frequency) {
  uint8_t value;
  if (kt0937_readRegister(0x000D, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  switch (frequency) {
    case 32768:
      value &= ~0x01;  // XTAL_SELビットをクリア
      break;
    case 38000:
      value |= 0x01;  // XTAL_SELビットをセット
      break;
    default:
      if (frequency >= 30000 && frequency <= 40000000) {
        value |= 0x10;  // RCLK_ENビットをセット
        uint32_t fpfd = frequency / 16;
        if (kt0937_writeRegister(0x0008, (fpfd >> 16) & 0x0F) != KT0937_D8_Error::OK ||
            kt0937_writeRegister(0x0009, (fpfd >> 8) & 0xFF) != KT0937_D8_Error::OK ||
            kt0937_writeRegister(0x000A, fpfd & 0xFF) != KT0937_D8_Error::OK) {
          return KT0937_D8_Error::ERROR_I2C;
        }
      } else {
        return KT0937_D8_Error::ERROR_PARAM;
      }
  }

  return kt0937_writeRegister(0x000D, value);
}

// デエンファシス設定
KT0937_D8_Error kt0937_setDeemphasis(bool is_75us) {
  uint8_t value;
  if (kt0937_readRegister(REG_DSPCFG1, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  if (is_75us) {
    value &= ~0x08;  // DEビットをクリア
  } else {
    value |= 0x08;  // DEビットをセット
  }

  return kt0937_writeRegister(REG_DSPCFG1, value);
}

// アンテナチューニング実行
KT0937_D8_Error kt0937_performAntennaTuning() {
  uint8_t value;
  if (kt0937_readRegister(0x002F, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  value |= 0x20;  // ANT_CALI_SWITCH_BAND ビットをセット
  if (kt0937_writeRegister(0x002F, value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  // チューニング完了を待つ
  unsigned long startTime = millis();
  while (millis() - startTime < 1000) {  // 1秒タイムアウト
    if (kt0937_readRegister(0x002F, &value) != KT0937_D8_Error::OK) {
      return KT0937_D8_Error::ERROR_I2C;
    }
    if (!(value & 0x20)) {
      return KT0937_D8_Error::OK;  // チューニング完了
    }
    delay(10);
  }

  return KT0937_D8_Error::ERROR_TIMEOUT;
}

// キャリア周波数オフセット取得
KT0937_D8_Error kt0937_getCarrierFrequencyOffset(int16_t *offset) {
  uint8_t value;
  KT0937_D8_Error err = kt0937_readRegister(REG_AFC_STATUS1, &value);
  if (err != KT0937_D8_Error::OK) {
    return err;
  }

  *offset = (int8_t)value * 1024;  // 1024Hzステップ
  return KT0937_D8_Error::OK;
}

// ソフトミュート設定
KT0937_D8_Error kt0937_configureSoftMute(uint8_t start_rssi, uint8_t slope_rssi, uint8_t start_snr, uint8_t slope_snr, uint8_t min_gain) {
  if (kt0937_writeRegister(REG_SOFTMUTE2, (start_rssi << 4) | slope_rssi) != KT0937_D8_Error::OK ||
      kt0937_writeRegister(REG_SOFTMUTE5, start_snr) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  uint8_t value;
  if (kt0937_readRegister(REG_BANDCFG3, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  value = (value & 0xC3) | (min_gain << 2);
  return kt0937_writeRegister(REG_BANDCFG3, value);
}

// シーク開始
KT0937_D8_Error kt0937_startSeek(bool up, bool wrap) {
  uint8_t value;
  if (kt0937_readRegister(REG_FMCHAN0, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  value |= 0x01;  // シーク開始ビットをセット
  if (up) {
    value |= 0x02;  // アップ方向にセット
  } else {
    value &= ~0x02;  // ダウン方向にセット
  }
  if (wrap) {
    value |= 0x04;  // ラップアラウンドを有効化
  } else {
    value &= ~0x04;  // ラップアラウンドを無効化
  }

  return kt0937_writeRegister(REG_FMCHAN0, value);
}

// シーク状態確認
KT0937_D8_Error kt0937_checkSeekStatus(bool *complete, bool *failed) {
  uint8_t value;
  if (kt0937_readRegister(REG_STATUS0, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  *complete = (value & 0x20) ? true : false;
  *failed = (value & 0x10) ? true : false;

  return KT0937_D8_Error::OK;
}

// RDS設定
KT0937_D8_Error kt0937_setRDS(bool enable) {
  uint8_t value;
  if (kt0937_readRegister(REG_DSPCFG1, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  if (enable) {
    value |= 0x10;  // RDSビットをセット
  } else {
    value &= ~0x10;  // RDSビットをクリア
  }

  return kt0937_writeRegister(REG_DSPCFG1, value);
}

// RDS読み取り
KT0937_D8_Error kt0937_readRDS(uint16_t *block_a, uint16_t *block_b, uint16_t *block_c, uint16_t *block_d) {
  uint8_t value_low, value_high;

  if (kt0937_readRegister(0x00CE, &value_low) != KT0937_D8_Error::OK || kt0937_readRegister(0x00CF, &value_high) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }
  *block_a = (value_high << 8) | value_low;

  if (kt0937_readRegister(0x00D0, &value_low) != KT0937_D8_Error::OK || kt0937_readRegister(0x00D1, &value_high) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }
  *block_b = (value_high << 8) | value_low;

  if (kt0937_readRegister(0x00D2, &value_low) != KT0937_D8_Error::OK || kt0937_readRegister(0x00D3, &value_high) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }
  *block_c = (value_high << 8) | value_low;

  if (kt0937_readRegister(0x00D4, &value_low) != KT0937_D8_Error::OK || kt0937_readRegister(0x00D5, &value_high) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }
  *block_d = (value_high << 8) | value_low;

  return KT0937_D8_Error::OK;
}

// 3Dサウンド設定
KT0937_D8_Error kt0937_set3DSound(uint8_t level) {
  if (level > 3) return KT0937_D8_Error::ERROR_PARAM;

  uint8_t value;
  if (kt0937_readRegister(REG_SOUNDCFG, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  value = (value & 0xF3) | (level << 2);
  return kt0937_writeRegister(REG_SOUNDCFG, value);
}

// 低電力モード設定
KT0937_D8_Error kt0937_setLowPowerMode(bool enable) {
  uint8_t value;
  if (kt0937_readRegister(REG_RXCFG0, &value) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  if (enable) {
    value |= 0x40;  // 低電力モードビットをセット
  } else {
    value &= ~0x40;  // 低電力モードビットをクリア
  }

  return kt0937_writeRegister(REG_RXCFG0, value);
}

// チップ情報取得
KT0937_D8_Error kt0937_getChipInfo(uint16_t *chip_id, uint8_t *version) {
  uint8_t value_low, value_high;

  if (kt0937_readRegister(REG_DEVICEID0, &value_low) != KT0937_D8_Error::OK || kt0937_readRegister(0x0001, &value_high) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }
  *chip_id = (value_high << 8) | value_low;

  if (kt0937_readRegister(0x0001, version) != KT0937_D8_Error::OK) {
    return KT0937_D8_Error::ERROR_I2C;
  }

  return KT0937_D8_Error::OK;
}

// エラーメッセージを文字列に変換する関数（続き）
const char* kt0937_errorToString(KT0937_D8_Error error) {
  switch (error) {
    case KT0937_D8_Error::OK:
      return "OK";
    case KT0937_D8_Error::ERROR_I2C:
      return "I2C Error";
    case KT0937_D8_Error::ERROR_PARAM:
      return "Invalid Parameter";
    case KT0937_D8_Error::ERROR_TIMEOUT:
      return "Timeout";
    case KT0937_D8_Error::ERROR_DEVICE:
      return "Device Error";
    default:
      return "Unknown Error";
  }
}

// デバッグ情報出力
void kt0937_printDebugInfo(void (*print_func)(const char*)) {
  char buffer[50];
  uint8_t value;

  print_func("KT0937-D8 Debug Info:");

  if (kt0937_readRegister(REG_STATUS8, &value) == KT0937_D8_Error::OK) {
    snprintf(buffer, sizeof(buffer), "RSSI: %d dBm", -110 + value);
    print_func(buffer);
  }

  if (kt0937_readRegister(REG_STATUS4, &value) == KT0937_D8_Error::OK) {
    snprintf(buffer, sizeof(buffer), "SNR: %d", value);
    print_func(buffer);
  }

  if (kt0937_readRegister(REG_STATUS0, &value) == KT0937_D8_Error::OK) {
    snprintf(buffer, sizeof(buffer), "Stereo: %s", (value & 0x01) ? "Yes" : "No");
    print_func(buffer);
    snprintf(buffer, sizeof(buffer), "Valid Channel: %s", (value & 0x04) ? "Yes" : "No");
    print_func(buffer);
  }

  if (kt0937_readRegister(REG_FMCHAN0, &value) == KT0937_D8_Error::OK) {
    snprintf(buffer, sizeof(buffer), "Mode: %s", (value & 0x40) ? "AM" : "FM");
    print_func(buffer);
  }

  int16_t offset;
  if (kt0937_getCarrierFrequencyOffset(&offset) == KT0937_D8_Error::OK) {
    snprintf(buffer, sizeof(buffer), "Carrier Frequency Offset: %d Hz", offset);
    print_func(buffer);
  }
}

// レジスタ内容表示
void kt0937_printRegister(uint8_t reg, void (*print_func)(const char*)) {
  uint8_t value;
  char buffer[50];
  
  if (kt0937_readRegister(reg, &value) == KT0937_D8_Error::OK) {
    snprintf(buffer, sizeof(buffer), "Register 0x%02X: 0x%02X", reg, value);
    print_func(buffer);
  } else {
    snprintf(buffer, sizeof(buffer), "Failed to read register 0x%02X", reg);
    print_func(buffer);
  }
}

// 全レジスタ内容表示
void kt0937_dumpAllRegisters(void (*print_func)(const char*)) {
  print_func("KT0937-D8 Register Dump:");
  for (uint16_t reg = 0x0000; reg <= 0x00EA; reg++) {
    kt0937_printRegister(reg, print_func);
  }
}

KT0937_D8_Error kt0937_setDetailedSpacing(KT0937_D8_Mode mode, uint8_t spacing) {
    uint8_t value;
    if (kt0937_readRegister(REG_BANDCFG2, &value) != KT0937_D8_Error::OK) {
        return KT0937_D8_Error::ERROR_I2C;
    }

    switch (mode) {
        case KT0937_D8_Mode::FM:
            value = (value & 0xCF) | ((spacing & 0x03) << 4); // FM_SPACE bits
            break;
        case KT0937_D8_Mode::MW:
            value = (value & 0xFC) | (spacing & 0x03); // MW_SPACE bits
            break;
        case KT0937_D8_Mode::SW:
            if (kt0937_readRegister(REG_BANDCFG3, &value) != KT0937_D8_Error::OK) {
                return KT0937_D8_Error::ERROR_I2C;
            }
            value = (value & 0xFC) | (spacing & 0x03); // SW_SPACE bits
            break;
        default:
            return KT0937_D8_Error::ERROR_PARAM;
    }

    return kt0937_writeRegister(mode == KT0937_D8_Mode::SW ? REG_BANDCFG3 : REG_BANDCFG2, value);
}

// オーディオ品質制御の実装
KT0937_D8_Error kt0937_setAudioQuality(const AudioQualityConfig& config) {
    KT0937_D8_Error err;

    // ステレオブレンド設定
    err = kt0937_writeRegister(REG_DSPCFG2, 
        (config.stereoBlendStart << 4) | (config.stereoBlendStop & 0x0F));
    if (err != KT0937_D8_Error::OK) return err;

    // ソフトミュート設定
    err = kt0937_writeRegister(REG_SOFTMUTE2,
        (config.softmuteStartRSSI << 4) | (config.softmuteSlopeRSSI & 0x07));
    if (err != KT0937_D8_Error::OK) return err;

    err = kt0937_writeRegister(REG_SOFTMUTE5,
        (config.softmuteStartSNR << 2) | (config.softmuteSlopeSNR & 0x03));
    if (err != KT0937_D8_Error::OK) return err;

    // バス設定
    err = kt0937_writeRegister(REG_SOUNDCFG,
        (config.bassBoost << 4) & 0x30);
    if (err != KT0937_D8_Error::OK) return err;

    // オーディオゲイン設定
    return kt0937_writeRegister(REG_DSPCFG0,
        (config.audioGain << 4) & 0x70);
}

// 保護機能の実装
KT0937_D8_Error kt0937_getProtectionStatus(ProtectionStatus* status) {
    uint8_t value;
    
    // 電源電圧監視
    if (kt0937_readRegister(0x0071, &value) != KT0937_D8_Error::OK) {
        return KT0937_D8_Error::ERROR_I2C;
    }
    status->voltage = (value & 0x7F) * 20 + 2100; // mV
    status->voltageLow = (value & 0x80) != 0;

    // 温度監視
    if (kt0937_readRegister(0x0072, &value) != KT0937_D8_Error::OK) {
        return KT0937_D8_Error::ERROR_I2C;
    }
    status->temperature = ((int8_t)value) - 40; // °C
    status->temperatureHigh = (status->temperature > 85);

    // アンテナ状態確認
    if (kt0937_readRegister(0x00EA, &value) != KT0937_D8_Error::OK) {
        return KT0937_D8_Error::ERROR_I2C;
    }
    status->antennaError = (value & 0x40) != 0;

    return KT0937_D8_Error::OK;
}

// 自動チャンネル設定機能の実装
KT0937_D8_Error kt0937_autoTune(const AutoTuneConfig& config, AutoTuneResult* results, uint8_t maxResults, uint8_t* numFound) {
    KT0937_D8_Error err;
    *numFound = 0;

    // 初期周波数に設定
    err = kt0937_setFrequency(config.startFreq);
    if (err != KT0937_D8_Error::OK) return err;

    // シーク開始
    err = kt0937_startSeek(config.seekUp, config.wrapAround);
    if (err != KT0937_D8_Error::OK) return err;

    while (*numFound < maxResults) {
        bool complete = false, failed = false;
        
        // シーク完了待ち
        do {
            err = kt0937_checkSeekStatus(&complete, &failed);
            if (err != KT0937_D8_Error::OK) return err;
            delay(10);
        } while (!complete && !failed);

        if (failed) break;

        // 現在の周波数を取得
        uint32_t freq;
        err = kt0937_getFrequency(&freq);
        if (err != KT0937_D8_Error::OK) return err;

        // RSSI, SNRを取得
        int8_t rssi;
        uint8_t snr;
        err = kt0937_getRSSI(&rssi);
        if (err != KT0937_D8_Error::OK) return err;
        err = kt0937_getSNR(&snr);
        if (err != KT0937_D8_Error::OK) return err;

        // 条件を満たす場合は結果に追加
        if (rssi >= config.minRSSI && snr >= config.minSNR) {
            bool isValid;
            err = kt0937_isChannelValid(&isValid);
            if (err != KT0937_D8_Error::OK) return err;

            results[*numFound].frequency = freq;
            results[*numFound].rssi = rssi;
            results[*numFound].snr = snr;
            results[*numFound].isValid = isValid;
            (*numFound)++;
        }

        // 終了周波数に達したら終了
        if (freq >= config.endFreq) break;

        // 次のチャンネルへ
        err = kt0937_startSeek(config.seekUp, config.wrapAround);
        if (err != KT0937_D8_Error::OK) return err;
    }

    return KT0937_D8_Error::OK;
}
