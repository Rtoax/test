static Bool matches_version(char *min_version)
{
   int a1, a2, a3, g1, g2, g3;  // 'a' = actual;  'g' = given

   if (min_version == NULL)  return True;  // no version specified

   // get actual version number
   FILE *fp = fopen("/proc/sys/kernel/osrelease", "r");
   if (fp == NULL || fscanf(fp, "%d.%d.%d", &a1, &a2, &a3) != 3) return False;
   fclose(fp);

   // parse given version number
   if (sscanf(min_version, "%d.%d.%d", &g1, &g2, &g3) != 3) return False;

//   printf("actual %d %d %d\n", a1, a2,a3);
//   printf("given  %d %d %d\n", g1, g2,g3);

   if (a1 > g1) return True;
   if (a1 < g1) return False;
   if (a2 > g2) return True;
   if (a2 < g2) return False;
   if (a3 >= g3) return True;

   return False;
}
