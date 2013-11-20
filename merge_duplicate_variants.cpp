/* The MIT License

   Copyright (c) 2013 Adrian Tan <atks@umich.edu>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include "merge_duplicate_variants.h"

KHASH_SET_INIT_STR(sdict);
 
namespace
{

class Igor : Program
{
    public:

    ///////////
    //options//
    ///////////
    std::string input_vcf_file;
    std::string output_vcf_file;
    std::vector<GenomeInterval> intervals;
    bool merge_by_pos;

    ///////
    //i/o//
    ///////
    BCFSyncedReader *sr;
    BCFOrderedWriter *odw;

    std::vector<bcf1_t*> pool;
        
    /////////
    //stats//
    /////////
    uint32_t no_total_variants;
    uint32_t no_unique_variants;
    
    /////////
    //tools//
    /////////
    VariantManip *var_manip;

    Igor(int argc, char **argv)
    {
        version = "0.57";

        //////////////////////////
        //options initialization//
        //////////////////////////
        try
        {
            std::string desc = "Merges duplicate variants by position with the option of considering alleles.  (This just discards the duplicate variant that appears later in the VCF file)";

            TCLAP::CmdLine cmd(desc, ' ', version);
            VTOutput my;
            cmd.setOutput(&my);
            TCLAP::ValueArg<std::string> arg_intervals("i", "i", "intervals []", false, "", "str", cmd);
            TCLAP::ValueArg<std::string> arg_interval_list("I", "I", "file containing list of intervals []", false, "", "file", cmd);
            TCLAP::ValueArg<std::string> arg_output_vcf_file("o", "o", "output VCF file [-]", false, "-", "str", cmd);
            TCLAP::SwitchArg arg_merge_by_position("p", "merge-by-position", "Merge by position [false]", cmd, false);
            TCLAP::UnlabeledValueArg<std::string> arg_input_vcf_file("<in.vcf>", "input VCF file", true, "","file", cmd);

            cmd.parse(argc, argv);

            input_vcf_file = arg_input_vcf_file.getValue();
            output_vcf_file = arg_output_vcf_file.getValue();
            merge_by_pos = arg_merge_by_position.getValue();
            parse_intervals(intervals, arg_interval_list.getValue(), arg_intervals.getValue());
        }
        catch (TCLAP::ArgException &e)
        {
            std::cerr << "error: " << e.error() << " for arg " << e.argId() << "\n";
            abort();
        }
    };
 
    void initialize()
    {
        //////////////////////
        //i/o initialization//
        //////////////////////
        std::vector<std::string> vcf_files(1, input_vcf_file);
        sr = new BCFSyncedReader(vcf_files, intervals);
        odw = new BCFOrderedWriter(output_vcf_file);
        odw->set_hdr(sr->hdrs[0]);
        odw->write_hdr();
        
        ////////////////////////
        //stats initialization//
        ////////////////////////
        no_total_variants = 0;
        no_unique_variants = 0;

        ////////////////////////
        //tools initialization//
        ////////////////////////
    }
    
    void merge_duplicate_variants()
    {
        kstring_t var = {0, 0, 0};
        
        std::vector<bcfptr> current_recs;
        
        khash_t(sdict) *m = kh_init(sdict);
        khiter_t k;
        int32_t ret;
  
        while (sr->read_next_position(current_recs))
        {
            kh_clear(sdict, m);
			
			for (uint32_t i=0; i<current_recs.size(); ++i)
			{
			    bcf_get_variant(current_recs[i].h, current_recs[i].v, &var);
		        if (kh_get(sdict, m, var.s)==kh_end(m))
		        {
		            odw->write(current_recs[i].v);
		            kh_put(sdict, m, strdup(var.s), &ret);
		            ++no_unique_variants;
		        }
		        
		        ++no_total_variants;
			}
        }

        odw->close();    
    };

    void print_options()
    {
        std::clog << "mergedups v" << version << "\n\n";

        std::clog << "options:     input VCF file        " << input_vcf_file << "\n";
        std::clog << "         [o] output VCF file       " << output_vcf_file << "\n";
        std::clog << "         [p] merge by              " << (merge_by_pos ? "position" : "alleles") << "\n";
        if (intervals.size()!=0)
        {
            std::clog << "         [i] intervals             " << intervals.size() <<  " intervals\n";
        }
        std::clog << "\n";
    }

    void print_stats()
    {
        std::clog << "\nstats: Total number of observed variants   " << no_total_variants << "\n";
        std::clog <<   "       Total number of unique variants     " << no_unique_variants << "\n\n";
    };

    ~Igor() {};

    private:
};

}

void merge_duplicate_variants(int argc, char ** argv)
{
    Igor igor(argc, argv);
    igor.print_options();
    igor.initialize();
    igor.merge_duplicate_variants();
    igor.print_stats();
};
