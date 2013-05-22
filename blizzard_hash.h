/**
 *
 * dynamic & static hash combine together
 * hash function is the famouse blizzard hash function
 * use 8 byte store 2 hash instead of the original key
 * lock the hash bucket for thead safe
 *
 * @author HouRui
 * @since 2013-04-07
 *
 */

#ifndef _BLIZZARDMAP_H
#define _BLIZZARDMAP_H 1

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include <string>
#include <vector>
#include <algorithm>

#include <ext/hash_map>

// 此方法 bucket_cnt 必须为2的次方
#define hash2bucket(hash, bucket_cnt) ((hash) & ((bucket_cnt) - 1)) 

using namespace std;
using namespace __gnu_cxx;

typedef uint64_t hash_t;

const static unsigned int CRYPT_TABLE[] = {
  0x55C636E2,0x02BE0170,0x584B71D4,0x2984F00E,0xB682C809,0x91CF876B,0x775A9C24,0x597D5CA5,
  0x5A1AFEB2,0xD3E9CE0D,0x32CDCDF8,0xB18201CD,0x3CCE05CE,0xA55D13BE,0xBB0AFE71,0x9376AB33,
  0x848F645E,0x87E45A45,0x45B86017,0x5E656CA8,0x1B851A95,0x2542DBD7,0xAB4DF9E4,0x5976AE9B,
  0x6C317E7D,0xCDDD2F94,0x3C3C13E5,0x335B1371,0x31A592CA,0x51E4FC4C,0xF7DB5B2F,0x8ABDBE41,
  0x8BEAA674,0x20D6B319,0xDE6C9A9D,0xC5AC84E5,0x445A5FEB,0x94958CB0,0x1E7D3847,0xF35D29B0,
  0xCA5CCEDA,0xB732C8B5,0xFDCC41DD,0x0EDCEC16,0x9D01FEAE,0x1165D38E,0x9EE193C8,0xBF33B13C,
  0x61BC0DFC,0xEF3E7BE9,0xF8D4D4C5,0xC79B7694,0x5A255943,0x0B3DD20A,0x9D1AB5A3,0xCFA8BA57,
  0x5E6D7069,0xCB89B731,0x3DC0D15B,0x0D4D7E7E,0x97E37F2B,0xFEFC2BB1,0xF95B16B5,0x27A55B93,
  0x45F22729,0x4C986630,0x7C666862,0x5FA40847,0xA3F16205,0x791B7764,0x386B36D6,0x6E6C3FEF,
  0xC75855DB,0x4ABC7DC7,0x4A328F9B,0xCEF20C0F,0x60B88F07,0xF7BB4B8F,0x830B5192,0x94F711EC,
  0x20250752,0x399D21A3,0xE5C0840D,0xE76CFFA5,0x624FAB29,0x5DF133E6,0x83E0B9B8,0xC5796BFB,
  0x4A7AB2D0,0xBA59A821,0x03A81E4C,0xCD3ADFDB,0x32B26B8C,0x8E35C533,0x9E6300E9,0x8CF92AC5,
  0x880D18EB,0x131A53B3,0x2ED2DC64,0xB23257C1,0xA06450C1,0x1B92CB8E,0x72ED730E,0x19A685F0,
  0x82836483,0x42D94E8A,0xEE9BD6F6,0x556D0B6A,0xBA65589A,0xDE24CCE4,0x53329F6C,0xC754FE8B,
  0x503D2DC7,0x10027BA4,0xD3B60A8B,0x68E68D83,0x0A9128A9,0x595FA35F,0x0B03B5BE,0x150A45C4,
  0xB1629CCE,0xE5F7497B,0x8A7098A4,0xB8233E69,0x8EA0F978,0x5B579970,0xEAB14318,0x4B28B263,
  0xB6766CEF,0x06782877,0x155C6DD0,0xC711333C,0xF819CEDF,0x00EB1D68,0xD6FFFA6E,0x439E5962,
  0xD765D6DB,0xCB0BCEE9,0x6D3C5647,0x965466F3,0x0CA983C9,0x74ECC1CE,0xFC0563B6,0x42B08FEE,
  0xC5B38853,0xFE502CEB,0x7B432FAF,0xC309E610,0x2C3997D8,0x43774654,0x15BD9D2C,0xED6A420D,
  0xC7FF520C,0xB8A97FD1,0x5E4D60CC,0xB9738D11,0xDA2181FF,0x73AC2597,0x3A8EEC8D,0xAC85E779,
  0xF3F975D6,0xB9FE7B91,0x0F155D1E,0x2860B6DD,0x835977CB,0xB0607436,0x9CAB7F6B,0x8AB91186,
  0xC12B51E9,0x20084E8B,0x44BA8EAD,0xA542B130,0x82BCD5C4,0xCC747F4E,0x0F1909D8,0xDA242E1C,
  0x6F7D1AA0,0xD2626486,0x88D0781E,0xAB695CCD,0xFA569145,0xB4FEB55C,0xBE47E896,0xE70A7A88,
  0xD56185A2,0xACF4C871,0x09282332,0x1DDEEAA8,0x590C7ADB,0xF4A97667,0xBFD85705,0x0EA77CCC,
  0xA9F85364,0x83195869,0x8BFB041A,0xDB842F5C,0xD6F0F315,0xA7756EA7,0x0A51B439,0xA9EDF8A3,
  0xD9084E2F,0x827407F8,0xD4AC8284,0x09739D0D,0xB3BB6CFC,0xD539C77D,0x6BBC9AC0,0x35C641AA,
  0x934C96B0,0xD17AF317,0x29C6BAEF,0xB275CDAC,0xD72662DE,0x9F5C2544,0xC1A98F75,0xD98E8F9A,
  0x47BD5C86,0x70C610A6,0xB5482ED4,0x23B9C68C,0x3C1BAE66,0x69556E7F,0xD902F5E0,0x653D195B,
  0xDE6541FB,0x07BCC6AC,0xC6EE7788,0x801534D4,0x2C1F35C0,0xD9DE614D,0xBDCCAC85,0xB4D4A0DA,
  0x242D549B,0x9D964796,0xB9CEB982,0x59FA99A9,0xD8986CC1,0x9E90C1A1,0x01BBD82F,0xD7F1C5FD,
  0xDD847EBA,0x883D305D,0x25F13152,0x4A92694D,0x77F1E601,0x8024E6E7,0x02A5F53D,0x9C3EF4D9,
  0xAF403CCC,0xE2AD03C0,0x46EDF6EC,0x6F9BD3E6,0xCC24AD7A,0x47AFAB12,0x82298DF7,0x708C9EEC,
  0x76F8C1B1,0xB39459D2,0x3F1E26D9,0xE1811BE7,0x56ED1C4D,0xC9D18AF8,0xE828060E,0x91CADA2E,
  0x5CCBF9B7,0xF1A552D4,0x3C9D4343,0xE1008785,0x2ADFEEBF,0xF90240A0,0x3D08CCE7,0x426E6FB0,
  0x573C984F,0x13A843AE,0x406B7439,0x636085D9,0x5000BA9A,0xAD4A47AB,0xAF001D8D,0x419907AE,
  0x185C8F96,0xE5E9ED4D,0x61764133,0xD3703D97,0xAC98F0C6,0xDBC3A37C,0x85F010C4,0x90491E32,
  0xF12E18BF,0xC88C96E1,0xD3FBD6D9,0xE3C28B08,0xD5BF08CC,0xB1E78859,0x2546DDCF,0xB030B200,
  0xAAFD2811,0x55B22D21,0xD38BF567,0x469C7A2B,0x5AD05792,0xA1A5981E,0x7DFB8384,0x34D1CA0A,
  0x7EB0DBE0,0xD61CE0F6,0x398068B7,0xE6406D1F,0x95AE6B47,0xE4281230,0xB0843061,0xA70A3A68,
  0xE340F625,0x72DCBFFD,0x8EB8AFCD,0x18B6661F,0x17EF5A5C,0x000C5B22,0x6BA13836,0x6165E383,
  0x74481C5B,0xE56F0711,0xA26F5024,0x5FF22E60,0x31A5E829,0xA1094BF0,0xC680EC6C,0x8CF327D7,
  0xEBF1348A,0x6A227D2F,0x74065184,0x8DF65112,0x2BBD05EE,0xE4D00ED6,0x2980EE1A,0x6AE1DA73,
  0xE84614DA,0x6C9906AB,0xCF8E02DB,0xD3723E97,0x92F66CAF,0xAC8491C7,0xAEC65696,0xB98997CF,
  0xFA16C762,0x6D73C65F,0x205D22A6,0x4DD3AAA5,0x2DEB6BC0,0x9F37686C,0x71A5282B,0x376BB9E0,
  0x7FFF2A1B,0xDE67982F,0x9CBF33CE,0x2E6DAB37,0x6E3424B9,0x0EE143BC,0x832A60D9,0xBB6329E1,
  0x13F6BEFD,0x5965FB84,0xF60B233C,0x3D695183,0x433224A1,0xB5D9CAE5,0x82459BAB,0x9F21B311,
  0xAF6C5247,0xB447B13A,0x7B2676C3,0xC38979CD,0x8526AE25,0xC550AD5B,0x685099A7,0x65E9C2BD,
  0xE5C6DC36,0xE10B37A9,0x88016878,0xCE81D4E4,0x24D6FC80,0x4106152D,0x6D4F5F90,0xC4DC74BE,
  0xDB48676C,0x6CB569B7,0xF3BF598F,0x042B08D9,0x02CCB2DE,0xB1056F65,0x47994AF4,0xFA141BA4,
  0x9376AB2E,0x07A76737,0x75E7E6FC,0x449D80A1,0x03B7259D,0xF6DF358A,0x5A75D5B9,0x47286923,
  0x3B1A30EF,0xEEBE3D6A,0x9DB1AA00,0x007A90D9,0x24667071,0x019C73CF,0x69039BCD,0x95900744,
  0x6518B1EB,0x6905F202,0xEE3951B2,0xE141FCA9,0x797FA832,0x5A95E55B,0xD6263B15,0x5B61F394,
  0x897ACB1C,0x005F83A9,0x22420F71,0xF495176E,0x7E138F3D,0x1392E384,0x373BF7AA,0x8E512816,
  0xA960B3CA,0x0474D74C,0xFFACD6D7,0x2EF5ED9E,0x60992AAA,0x7E690E99,0x23C0749D,0xD8E29105,
  0x555D5909,0x15631BFE,0xA69C5A1C,0x501017CA,0x99438048,0x38733AC7,0xE682E2C8,0xD4655FD6,
  0x956E4C04,0x347DF643,0x2F4B177B,0x93ED3AA4,0xA77E1DD5,0x7AE55702,0xD2A52FD9,0xEF8BA18C,
  0xB7D3C1EE,0x8078BA8D,0xAB5AAADB,0x752BE08F,0x068B31C1,0x078AAE3C,0xAA5A8343,0x123D9268,
  0x2CEAEE43,0x8EBDB239,0x650251F3,0x04883648,0x8C62E12E,0x12B32167,0xE5112E9A,0x10002548,
  0x3E7A818D,0x077E5327,0xF140CC21,0x6CE7D75D,0x9B99F9A5,0x3215741C,0xB6AADBAE,0x738768DC,
  0x82A3742F,0x76517020,0xDD872AD8,0x9D0902B2,0x7D1A6B04,0x49381592,0x63A652A5,0x0C15E626,
  0xE22F70D6,0x01E84385,0xB29DE134,0x20C5000E,0xE961F443,0x2D31662E,0x3CE6BC28,0x34F9DD94,
  0xFA45DE53,0x497588BD,0x9468215B,0x0777FA5C,0x6F7114C0,0xE0E82694,0xE4371986,0x57112DE2,
  0xE0CAC289,0xF2A3CEE0,0x6A41E1B9,0xBFCEA77D,0xF927FD52,0x69747D98,0xBEA76CDB,0x8DD39557,
  0x04DB5ECE,0x2A0885C8,0x3BE4E8EE,0x21D785DC,0x09DE7C0E,0x3258EA33,0x51922982,0xEE8DD024
};

inline hash_t blizzard_hash(const char *key, const int section)
{
  unsigned char *str = (unsigned char *)key;
  unsigned int ch, seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;
  
  while (*str != 0) {
    ch = *str++;
    seed1 = CRYPT_TABLE[(section << 8) + ch] ^ (seed1 + seed2);
    seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
  }
  
  return seed1;
}

template <class key_t>
inline hash_t hashing(const key_t &key) {
  return (hash_t)key;
}

template <>
inline hash_t hashing<string>(const string &key) {
  int64_t h = blizzard_hash(key.c_str(), 0);
  h <<= 32;
  h |= blizzard_hash(key.c_str(), 1);
  return (hash_t)h;
}

struct header_t
{
  unsigned int bucket_num;
  unsigned int item_num;
};

template <class val_t>
struct node_t
{
  hash_t hash;
  val_t  value;
  bool operator < (const struct node_t<val_t> &node) const
  {
    return this->hash < node.hash;
  }
};
    
template <class val_t>
struct chain_t
{
  chain_t<val_t> *next;
  hash_t hash;
  val_t  value;
  chain_t():next(NULL) {}
  ~chain_t() {
    if (NULL != next) delete next;
  }
};
    
inline unsigned int calc_bucket_num(const unsigned int item_num)
{
  unsigned int bucket_pow = (unsigned int)log2(item_num);
  if (bucket_pow < 10) bucket_pow = 10;
  if (bucket_pow > 25) bucket_pow = 25;
  return 1 << bucket_pow;
}
    

template <class key_t, class val_t>
class BlizzardMap {
public:
  BlizzardMap():
  chain_size_(0) {}
  virtual ~BlizzardMap() {
    for (size_t i = 0; i < chain_.size(); ++i) {
      if (NULL == chain_[i]) continue;
      delete chain_[i];
    }
    for (size_t i = 0; i < lock_.size(); ++i) {
      pthread_rwlock_destroy(&lock_[i]);
    }
  }
  
protected:
  int chain_size_;
  
protected:
  struct header_t header_;
  
protected:
  vector<unsigned int>            bucket_;
  vector<hash_t>                  hasher_;
  vector<val_t>                   data_;
  vector<pthread_rwlock_t>        lock_;
  vector<struct chain_t<val_t> *> chain_;
  
public:
  int  size(void);
  bool unserialize(const string & filename);
  bool unserialize(FILE *fp);
  bool get(const key_t &key, val_t &value);
  
public:
  bool set(const key_t &key, const val_t &value);
  
protected:
  bool init(void);
  bool insertChain  (const hash_t &hash, const val_t &value);
  val_t *existsData (const hash_t &hash, unsigned int bucket_id);
  
protected:
  chain_t<val_t> * existsChain(const hash_t &hash, unsigned int bucket_id);
  
public:
  bool static serialize(hash_map<key_t, val_t> &hmap, FILE *fp);
};

template <class key_t, class val_t>
bool BlizzardMap<key_t, val_t>::serialize(hash_map<key_t, val_t> &hmap, FILE *fp)
{
  struct header_t header; memset(&header, 0, sizeof(struct header_t));
  
  vector<pair<key_t, val_t> > item_vec(hmap.begin(), hmap.end());
  
  header.item_num   = item_vec.size();
  header.bucket_num = calc_bucket_num(header.item_num);
  
  vector<vector<node_t<val_t> > > bucket(header.bucket_num);
  
  unsigned int bucket_id;
  node_t<val_t> cur_node;
  
  for (size_t i = 0; i < header.item_num; i++) {
    cur_node.hash  = hashing(item_vec[i].first);
    cur_node.value = item_vec[i].second;
    bucket_id = hash2bucket(cur_node.hash, header.bucket_num);
    bucket[bucket_id].push_back(cur_node);
  }
  
  // address begin from 1
  // if address = 0 the bucket is empty
  vector<unsigned int> bucket_address(header.bucket_num);
  
  vector<hash_t> pure_hash(header.item_num + 1);
  vector<val_t>  pure_value(header.item_num + 1);
  
  unsigned int offset = 1;
  
  for (size_t i = 0; i < bucket.size(); i++) {
    if (bucket[i].size() == 0) continue;
    bucket_address[i] = offset;
    sort(bucket[i].begin(), bucket[i].end());
    for (size_t j = 0; j < bucket[i].size(); j++) {
      pure_hash[offset]  = bucket[i][j].hash;
      pure_value[offset] = bucket[i][j].value;
      offset++;
    }
  }
  
  fwrite(&header, 1, sizeof(struct header_t), fp);
  fwrite(&bucket_address[0], bucket_address.size(), sizeof(unsigned int), fp);
  
  fwrite(&pure_hash[0], pure_hash.size(), sizeof(hash_t), fp);
  fwrite(&pure_value[0], pure_value.size(), sizeof(val_t), fp);
  
  return true;
  
}
    
template <class key_t, class val_t>
bool BlizzardMap<key_t, val_t>::init()
{
  
  // init lock & open chain
  chain_.resize(header_.bucket_num);
  if (chain_.size() != header_.bucket_num) return false;
  
  lock_.resize(header_.bucket_num);
  if (lock_.size() != header_.bucket_num) return false;
  
  for (size_t i = 0; i < lock_.size(); i++) {
    pthread_rwlock_init(&lock_[i], NULL);
  }
  return true;
}
    
template <class key_t, class val_t>
int BlizzardMap<key_t, val_t>::size(void)
{
  return header_.item_num + chain_size_;
}

template <class key_t, class val_t>
bool BlizzardMap<key_t, val_t>::unserialize(const string & filename)
{
  FILE * fp = fopen(filename.c_str(), "rb");
  if (NULL == fp) {
    fprintf(stderr, "file: %s, open error", filename.c_str());
    return false;
  }
  unserialize(fp);
  fclose(fp);
  return true;
}
    
template <class key_t, class val_t>
bool BlizzardMap<key_t, val_t>::unserialize(FILE *fp)
{
  do {
    size_t read_size;
    read_size = fread(&header_, sizeof(struct header_t), 1, fp);
    if (1 != read_size) break;

    bucket_.resize(header_.bucket_num);
    read_size = fread(&bucket_[0], sizeof(unsigned int), bucket_.size(), fp);
    if (bucket_.size() != read_size) break;
    
    // +1 for the gap
    hasher_.resize(header_.item_num + 1);
    read_size = fread(&hasher_[0], sizeof(hash_t), hasher_.size(), fp);
    if (hasher_.size() != read_size) break;

    // + 1 for the gap
    data_.resize(header_.item_num + 1);
    read_size = fread(&data_[0], sizeof(val_t), data_.size(), fp);
    if (data_.size() != read_size) break;
    
    if (!init()) break;
    
    return true;
    
  } while(0);
  
  return false;
}

template <class key_t, class val_t>
bool BlizzardMap<key_t, val_t>::get(const key_t &key, val_t &value)
{ 
  memset(&value, 0, sizeof(val_t));
  
  hash_t hash = hashing(key);
  unsigned int bucket_id = hash2bucket(hash, header_.bucket_num);
  
  // lock
  pthread_rwlock_t *lock = &lock_[bucket_id];
  pthread_rwlock_rdlock(lock);
  
  // data
  val_t * val = existsData(hash, bucket_id);
  if (NULL != val) {
    value = *val;
    pthread_rwlock_unlock(lock);
    return true;
  }
  
  // chain
  chain_t<val_t> * chain = existsChain(hash, bucket_id);
  if (NULL != chain && chain->hash == hash) {
    value = chain->value;
    // unlock
    pthread_rwlock_unlock(lock);
    return true;
  }
  
  // unlock
  pthread_rwlock_unlock(lock);
  return false;
}

template <class key_t, class val_t>
bool BlizzardMap<key_t, val_t>::insertChain(const hash_t &hash, const val_t &value)
{
  unsigned int bucket_id = hash2bucket(hash, header_.bucket_num);
  
  if (NULL == chain_[bucket_id]) {
    chain_[bucket_id] = new chain_t<val_t>;
    chain_[bucket_id]->hash  = hash;
    chain_[bucket_id]->value = value;
    return true;
  }
  
  struct chain_t<val_t> * next  = NULL;
  struct chain_t<val_t> * chain = existsChain(hash, bucket_id);
  
  // head pointer  
  if (NULL == chain || (chain == chain_[bucket_id] && chain->hash > hash)) {
    chain_[bucket_id] = new chain_t<val_t>;
    chain_[bucket_id]->hash  = hash;
    chain_[bucket_id]->value = value;
    chain_[bucket_id]->next  = chain;
    return true;
  }
  
  // equal
  if (chain->hash == hash) {
    chain->value = value; return true;
  }
  
  // other pointer
  next = new chain_t<val_t>;
  next->next = chain->next;
  chain->next = next;
  
  return true;
}
    
template <class key_t, class val_t>
val_t * BlizzardMap<key_t, val_t>::existsData(const hash_t &hash, unsigned int bucket_id)
{
  if (0 == bucket_[bucket_id]) return NULL;
  
  unsigned int offset = bucket_[bucket_id];
  while (offset <= header_.item_num) {
    if (hasher_[offset] == hash) return &data_[offset];
    if (hasher_[offset] > hash) break;
    if (bucket_id != hash2bucket(hasher_[offset], header_.bucket_num)) break;
    offset++;
  }
  return NULL;
}

template <class key_t, class val_t>
chain_t<val_t> * BlizzardMap<key_t, val_t>::existsChain(const hash_t &hash, unsigned int bucket_id)
{
  chain_t<val_t> *next    = NULL;
  chain_t<val_t> *current = chain_[bucket_id];
  
  // for the head pointer only
  if (NULL == current || current->hash >= hash) return current;
  
  while (NULL != (next = current->next)) {
    if (next->hash == hash) return next;
    if (next->hash > hash) break;
    current = next;
  }
  
  return current;
}
    
template <class key_t, class val_t>
bool BlizzardMap<key_t, val_t>::set(const key_t &key, const val_t &value)
{
  hash_t hash = hashing(key);
  unsigned int bucket_id = hash2bucket(hash, header_.bucket_num);
  
  // lock
  pthread_rwlock_t *lock = &lock_[bucket_id];
  pthread_rwlock_wrlock(lock);
  
  val_t * exists_value = existsData(hash, bucket_id);
  if (NULL != exists_value) {
    *exists_value = value;
    
    // unlock
    pthread_rwlock_unlock(lock);
    return true;
  }
  
  insertChain(hash, value);
  
  // unlock
  pthread_rwlock_unlock(lock);
  return true;
}
    
#endif /* _BLIZZARDMAP_H */
