#include <json.h>
#include <libintl.h>
#include <stdio.h>

/* Declare a country table with maximum possible size for alpha-2 codes
 * so that can quickly lookup country names:
 * AA = entry 0 ( 0*26 + 0)
 * AB = entry 1 ( 0*26 + 1)
 * .
 * .
 * GK = entry 166 (6*26 + 10)
 * HM = entry 194 (7*26 + 12)
 * .
 * .
 * ZY = entry 674 (
 * ZZ = entry 675 (25*26 + 25)
 */
static const char* country_table[26*26];

/* Convert an alpha-2 country code to an index number:
 * Assume A=0, B=1, C=2 ............etc.
 * index = 26*<1st_letter> + <2nd_letter>
 */
static inline int country_code_to_index(const char *cc)
{
	return 26*((*(short*)cc & 0x00FF)-65) + (((*(short*)cc & 0xFF00)>>8)-65);
}

static json_object *iso3166dash1_file_jobj;

int initCountryTable(void)
{
	json_object *iso3166dash1_jobj;
	int i;
	int cti;

	/* Load ISO 3166-1 table in JSON format from file system */
	iso3166dash1_file_jobj = json_object_from_file("/usr/share/iso-codes/json/iso_3166-1.json");

	/* Get the json array of countries */
	json_object_object_get_ex(iso3166dash1_file_jobj, "3166-1", &iso3166dash1_jobj);

	for (i=0; i<json_object_array_length(iso3166dash1_jobj); i++) {
		json_object *country_jobj;
		json_object *alpha2_jobj;
		json_object *name_jobj;

		country_jobj = json_object_array_get_idx(iso3166dash1_jobj, i);
		json_object_object_get_ex(country_jobj, "alpha_2", &alpha2_jobj);
		json_object_object_get_ex(country_jobj, "name", &name_jobj);

		cti = country_code_to_index(json_object_get_string(alpha2_jobj));
		country_table[cti] = json_object_get_string(name_jobj);
	}

	return 0;
}

_Bool isValidCountryCode(const char *cc)
{
	int first;
	int second;

	first = (*(short*)cc & 0x00FF)-65;
	second = ((*(short*)cc & 0xFF00)>>8)-65;

	if (first < 0 || first > 25)
		return FALSE;
	if (second < 0 || second > 25)
		return FALSE;

	return TRUE;
}

const char* getCountryName(const char *code)
{
	return country_table[country_code_to_index(code)];
}

void cleanupCountryTable(void)
{
	json_object_put(iso3166dash1_file_jobj);
}
