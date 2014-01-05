/********************************************************************************/
#ifndef _TIME_ZONE_h
#define _TIME_ZONE_h
#define GMT_TZ_DESCR "Greenwich Mean Time: London"


static struct {
	int		gmt_offset;
	char	*name;
	char	dst_idx;
} time_zones[] = {
	{-12,	"Eniwetok", 0},							// daylight saving rule not found
//	{-12,	"Kwajalein"},
	{-11,	"Midway Island", 0},					// daylight saving rule not found
//	{-11,	"Samoa"},
	{-10,	"Hawaii", 0},							// daylight saving rule not found
	{-9,	"Alaska", 0},							// daylight saving rule not found
	{-8,	"Pacific Time (US & Canada)", 3},		// 03:00 Sun 03 Apr 2005 - 02:00 Sun 30 Oct 2005
//	{-8,	"Tijuana"},
//	{-7,	"Arizona"},
	{-7,	"Mountain Time (US & Canada)", 3},		//  03:00 Sun 03 Apr 2005 - 02:00 Sun 30 Oct 2005
//	{-6,	"Cental America"},
	{-6,	"Central Time (US & Canada)", 3},		// 03:00 Sun 03 Apr 2005 - 02:00 Sun 30 Oct 2005
	{-6,	"Mexico City", 3},						// 03:00 Sun 03 Apr 2005 - 02:00 Sun 30 Oct 2005
//	{-6,	"Saskatchewan"},
	{-5,	"Bogota", 0},
	{-5,	"Lima", 0},
//	{-5,	"Quito"},
	{-5,	"Eastern Time (US & Canada)", 3},		// 03:00 Sun 03 Apr 2005 - 02:00 Sun 30 Oct 2005
//	{-5,	"Indiana (East)"},
	{-4,	"Atlantic Time (Canada)", 3},			// 03:00 Sun 03 Apr 2005 - 02:00 Sun 30 Oct 2005
	{-4,	"Caracas", 0},
	{-4,	"La Paz", 0},
//	{-4,	"Santiago"},			// 00:00 Sun 13 Mar 2005 - 00:00 Sun 09 Oct 2005
//	{-3,	"Newfoundland"},
//	{-3,	"Brasilia"},			// 00:00 Sun 16 Oct 2005 - 00:00 Sun 20 Feb 2005
	{-3,	"Buenos Aires", 0},
//	{-3,	"Georgetown"},
//	{-3,	"Greenland"},
	{-2,	"Mid-Atlantic", 0},						// daylight saving rule not found
	{-1,	"Azores", 0},							// daylight saving rule not found
//	{-1,	"Cape Verde Is."},
//	{0,		"Casablanca"},
//	{0,		"Monrovia"},
	{0,		GMT_TZ_DESCR, 0},
//	{0,		"Dublin"},
//	{0,		"Edinburgh"},
//	{0,		"Lisbon"},
	{1,		"Amsterdam", 1},						// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
	{1,		"Berlin", 1},							// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
//	{1,		"Bern"},
	{1,		"Rome", 1},								// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
	{1,		"Stockholm", 1},						// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
	{1,		"Vienna", 1},							// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
	{1,		"Belgrade", 1},							// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
//	{1,		"Bratislava"},
	{1,		"Budapest", 1},							// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
//	{1,		"Ljubljana"},
	{1,		"Prague", 1},							// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
	{1,		"Brussels", 1},							// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
	{1,		"Copenhagen", 1},						// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
	{1,		"Madrid", 1},							// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
	{1,		"Paris", 1},							// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
//	{1,		"Sarajevo"},
//	{1,		"Skopje"},
//	{1,		"Sofija"},
//	{1,		"Vilnius"},
	{1,		"Warsaw", 1},							// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
	{1,		"Zagreb", 1},							// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
//	{1,		"West Central Africa"},
	{2,		"Athens", 2},							// 03:00 Sun 27 Mar 2005 - 04:00 Sun 30 Oct 2005
	{2,		"Istanbul", 2},							// 03:00 Sun 27 Mar 2005 - 04:00 Sun 30 Oct 2005
	{2,		"Minsk", 2},							// 03:00 Sun 27 Mar 2005 - 04:00 Sun 30 Oct 2005
	{2,		"Bucharest", 2},						// 03:00 Sun 27 Mar 2005 - 04:00 Sun 30 Oct 2005
//	{2,		"Cairo", },			// 00:00 Fri 29 Apr 2005 - 00:00 Fri 30 Sep 2005
	{2,		"Harare", 0},
//	{2,		"Pretoria"},
	{2,		"Helsinki", 2},							// 03:00 Sun 27 Mar 2005 - 04:00 Sun 30 Oct 2005
//	{2,		"Riga"},
	{2,		"Tallinn", 2},							// 03:00 Sun 27 Mar 2005 - 04:00 Sun 30 Oct 2005
//	{2,		"Jerusalem"},
//	{3,		"Baghdad"},			// 03:00 Fri 01 Apr 2005 - 04:00 Sat 01 Oct 2005
	{3,		"Kuwait", 0},
	{3,		"Riyadh", 0},
	{3,		"Moscow", 1},							// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
//	{3,		"St.Petersburg"},
//	{3,		"Volgograd"},
	{3,		"Nairobi", 0},
//	{3,		"Tehran"},
	{4,		"Abu Dhabi", 0},
//	{4,		"Muscat"},
//	{4,		"Baku"},
//	{4,		"Tbilisi"},
//	{4,		"Yerevan"},
	{4.5,	"Kabul", 0},
//	{5,		"Ekaterinburg"},
	{5,		"Islamabad", 0},
	{5,		"Karachi", 0},
	{5,		"Tashkent", 0},
//	{5,		"Calcutta"},
//	{5,		"Chennai"},
	{5.5,	"Mumbai", 0},
	{5.5,	"New Delhi", 0},
	{5.75,	"Kathmandu", 0},
//	{6,		"Almaty"},
//	{6,		"Novosibirsk"},
//	{6,		"Astana"},
	{6,		"Dhaka", 0},
//	{6,		"Sri Jayawardenepura"},
//	{6,		"Rangoon"},
	{7,		"Bangkok", 0},
	{7,		"Hanoi", 0},
	{7,		"Jakarta", 0},
//	{7,		"Krasnoyarsk"},
	{8,		"Beijing", 0},
//	{8,		"Chongqing"},
	{8,		"Hong Kong", 0},
//	{8,		"Urumqi"},
//	{8,		"Irkutsk"},
//	{8,		"Ulaann Bataar"},
	{8,		"Kuala Lumpur", 0},
	{8,		"Singapore", 0},
	{8,		"Perth", 0},
	{8,		"Taipei", 0},
	{9,		"Osaka", 0},
//	{9,		"Sapporo"},
	{9,		"Tokyo", 0},
	{9,		"Seoul", 0},
//	{9,		"Yakutsk"},
//	{9,		"Adelaide"},
	{9.5,	"Darwin", 0},
	{10,	"Brisbane", 0},
//	{10,	"Guam"},
//	{10,	"Port Moresby"},
//	{10,	"Hobart"},
	{10,	"Vladivostok", 1},						// 02:00 Sun 27 Mar 2005 - 03:00 Sun 30 Oct 2005
	{11,	"Canberra", 4},							// 02:00 Sun 30 Oct 2005 - 03:00 Sun 27 Mar 2005
	{11,	"Sydney", 4},							// 02:00 Sun 30 Oct 2005 - 03:00 Sun 27 Mar 2005
	{11,	"Melbourne", 4},						// 02:00 Sun 30 Oct 2005 - 03:00 Sun 27 Mar 2005
//	{11,	"Magadan"},
//	{11,	"Solomon Is."},
//	{11,	"New Caledonia"},
	{12,	"Auckland", 0},							// daylight saving rule not found
//	{12,	"Wellington"},			// 02:00 Sun 02 Oct 2005 - 03:00 Sun 20 Mar 2005
//	{12,	"Fiji"},
//	{12,	"Kamchatka"},
//	{12,	"Marshall Is."},
	{13,	"Nuku'alofa", 0},						// daylight saving rule not found
    {0,		NULL}
};

#if 0
struct dstTable
{
	unsigned char	start_day[21];
	unsigned char	end_day[21];
	unsigned char	start_month;
	unsigned char	end_month;
	unsigned char	start_time;
	unsigned char	end_time;
} dstEntry[] =
{
	{{ }, { }, 0, 0, 0, 0},
	// Mar last Sun on 02:00  - Oct last Sun on 03:00 (2005-2025)
	{{27,26,25,30,29,28,27,25,31,30,29,27,26,25,31,29,28,27,26,31,30}, {30,29,28,26,25,31,30,28,27,26,25,30,29,28,27,25,31,30,29,27,26}, 3, 10, 2, 3},
	// Mar last Sun on 03:00  - Oct last Sun on 04:00 (2005-2025)
	{{27,26,25,30,29,28,27,25,31,30,29,27,26,25,31,29,28,27,26,31,30}, {30,29,28,26,25,31,30,28,27,26,25,30,29,28,27,25,31,30,29,27,26}, 3, 10, 3, 4},
	// Apr first Sun on 03:00 - Oct last Sun on 02:00 (2005-2025)
	{{3,2,1,6,5,4,3,1,7,6,5,3,2,1,7,5,4,3,2,7,6,}, {30,29,28,26,25,31,30,28,27,26,25,30,29,28,27,25,31,30,29,27,26}, 4, 10, 3, 2},
	// Oct last Sun on 02:00 - Mar last Sun on 03:00 (2005-2025)
	{{30,29,28,26,25,31,30,28,27,26,25,30,29,28,27,25,31,30,29,27,26}, {27,26,25,30,29,28,27,25,31,30,29,27,26,25,31,29,28,27,26,31,30}, 10, 3, 2, 3},

};
#endif
#endif


