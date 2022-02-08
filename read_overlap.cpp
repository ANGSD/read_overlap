#include <cstdio>
#include <cstring>
#include <map>
#include <htslib/hts.h>
#include <htslib/sam.h>
#include <vector>

struct cmp_str
{
   bool operator()(char const *a, char const *b) const
   {
      return std::strcmp(a, b) < 0;
   }
};

typedef std::map<char *,int,cmp_str> myMap;

myMap amap;

void hts(char *fname,int filenr){
  samFile *fp_in = hts_open(fname,"r");
  bam_hdr_t *hdr = NULL;
  hdr=sam_hdr_read(fp_in);
  bam1_t *aln = bam_init1();
  assert(hdr);

  while(sam_read1(fp_in,hdr,aln) >= 0) {
    char *qname = bam_get_qname(aln);
    myMap::iterator it = amap.find(qname);
    if(it==amap.end())
      amap[strdup(qname)] = filenr;
    else
      it->second = it->second |filenr;
  }
  //should clean up fp_in, hdr and aln. But we dont care for this small program
  fprintf(stderr,"\t-> mapstructure has: %lu number of elements after reading file: \'%s\'\n",amap.size(),fname);
}

int main(int argc, char **argv){
  char *bam1 = argv[1];
  char *bam2 = argv[2];
  char *bam3 = argv[3];
  hts(bam1,1);
  hts(bam2,2);
  hts(bam3,4);

  std::map<int,size_t> counts;

  for(myMap::iterator it=amap.begin();it!=amap.end();it++){
    std::map<int,size_t>::iterator it2 = counts.find(it->second);
    if(it2==counts.end())
      counts[it->second] =1;
    else
      it2->second = it2->second +1;
  }

  fprintf(stdout,"\t-> Number of combinations: %lu\n",counts.size());
  size_t results[8]={0,0,0,0,0,0,0,0};
  for(std::map<int,size_t>::iterator it2=counts.begin();it2!=counts.end();it2++)
    results[it2->first] = it2->second;
  for(int i=1;i<8;i++)
    fprintf(stdout,"%d\t%lu\n",i,results[i]);
  fprintf(stderr,"private to \'%s\': %lu\n",bam1,results[1]);
  fprintf(stderr,"private to \'%s\': %lu\n",bam2,results[2]);
  fprintf(stderr,"private to \'%s\': %lu\n",bam3,results[4]);
  fprintf(stderr,"shared between \'%s\',\'%s\': %lu\n",bam1,bam2,results[3]);
  fprintf(stderr,"shared between \'%s\',\'%s\': %lu\n",bam1,bam3,results[5]);
  fprintf(stderr,"shared between \'%s\',\'%s\': %lu\n",bam2,bam3,results[6]);
  fprintf(stderr,"shared between \'%s\',\'%s\',\'%s\': %lu\n",bam1,bam2,bam3,results[7]);
  return 0;
}
