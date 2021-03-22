/*
 * brzo_i2c_scanner
 * 
 * @brief A tool write for ESP8266 to scan I2C buss for connected devices using brzo_i2c lib.
 * 
 * Uses brzo_i2c lib by pasko-zh
 * https://github.com/pasko-zh/brzo_i2c
 * 
 * @author MichoP
 * https://github.com/MichoP/brzo_i2c_scanner
 * 
 * brzo_i2c_scaner is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*/

#include <Arduino.h>
#include <brzo_i2c.h>

#define ADDR_start 1
#define ADDR_end 127
// TODO: add suport for 11-bit addreses

// Select errors to report
// #define ER1
// #define ER2
#define ER4
#define ER8
#define ER16
#define ER32

const uint8_t SDA_PIN = D2;
const uint8_t SCL_PIN = D1;
const uint8_t SCL_speed = 200;
// TODO: add multiple speed testing
const uint32_t SCL_STRETCH_TIMEOUT = 50000;

void setup() {
  delay(5000);
  Serial.begin(115200);
  Serial.println("\n====================================");
  Serial.println("========= brzo_i2c_scanner =========");
  Serial.println("============= v 0.1 ================");

}

void loop() {
  uint8_t error, address;
  uint8_t nDevice = 0;
  uint8_t fa_index = 0;
  uint8_t fAddreses[127];
  uint8_t tData[] = {0};

  bool sc = false;

  Serial.println("============== START ===============");
  Serial.println("Scanning now.");
  brzo_i2c_setup(SDA_PIN, SCL_PIN, SCL_STRETCH_TIMEOUT);
  delay(100);

  for(address = ADDR_start; address < ADDR_end; address ++){
    // It is mandatory perform send or write command within transmission. 
    // Otherwise the transmission will not accrue
    brzo_i2c_start_transaction(address, SCL_speed);
    brzo_i2c_write(tData, 1, false);
    error = brzo_i2c_end_transaction();
    // TODO: better chceck algorytm

    bool found = true;

    switch(error){
      case 0:
        // All i2c commands were executed without errors
        Serial.print("\u2713 | I2C device found      | 0|\u2713|0x");
        fAddreses[fa_index] = address;
        fa_index ++;
        nDevice ++;
      break;
      case 1:
        // Bus not free, i.e. either SDA or SCL is low
        sc = true;
        #if defined(ER1)
          Serial.print("! | Bus not free          | 1|!|0x");
        #else
          found = false;
        #endif
      break;
      case 2:
        // Not ACK ("NACK") by slave during write: 
        // Either the slave did not respond to the given slave address; 
        // or the slave did not ACK a byte transferred by the master.
        
        #if defined(ER2)
          Serial.print("! | Not ACK - write       | 2|!|0x");
        #else
          found = false;
        #endif
      break;
      case 4:
        // Not ACK ("NACK") by slave during read, i.e. slave did not respond to the given slave address
        #if defined(ER4)
          Serial.print("! | Not ACK - read        | 4|!|0x");
        #else
          found = false;
        #endif
      break;
      case 8:
        // Clock Stretching by slave exceeded maximum clock stretching time. Most probably, there is a bus stall now!
        #if defined(ER8)
          Serial.print("! | CLK Stretching        | 8|!|0x");
        #else
          found = false;
        #endif
      break;
      case 16:
        // Read was called with 0 bytes to be read by the master. 
        // Command not sent to the slave, since this could yield to a bus stall
        #if defined(ER16)
          Serial.print("! | R called with 0 bytes |16|!|0x");
        #else
          found = false;
        #endif
      break;
      case 32:
        // ACK Polling timeout exceeded
        #if defined(ER32)
          Serial.print("! | ACK timeout exceeded  |32|!|0x");
        #else
          found = false;
        #endif
      break;
    }

    if(found){
      // Ptinting all found errors
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if(nDevice > 0){
    Serial.print("=======End of scan. Found: ");
    Serial.print(nDevice);
    if(nDevice < 10)
      Serial.println("========");
    else if(nDevice < 100)
      Serial.println("=======");
    else
      Serial.println("======");
    for(uint8_t i = 0; i < nDevice; i++){
      Serial.print("| 0x");
      if (fAddreses[i]<16)
        Serial.print("0");
      Serial.println(fAddreses[i],HEX);
    }
  }
  else{
    Serial.println("========== no device found! ========");
  }
  if(sc){
    Serial.println("############# WARNING ##############");
    Serial.println("!Buss not free detected");
    Serial.println("!Short circute possible");
  }
  Serial.println("=============== END ================");
  delay(5000);
}