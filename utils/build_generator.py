
import datetime
import sys
import subprocess
import base64

#print(sys.argv[1])

versionFileName = sys.argv[1] + "/src/version.cpp"
buildVersionDef = "#define BUILD_VERSION"

def parseBuildVersion(fileName):
   try:
      with open(fileName, 'r') as f:
          for str in f.readlines():
              if str.startswith(buildVersionDef):
                  res = str.replace(buildVersionDef, "")
                  res = res.replace("\"","").strip()
                  return int(res)
   except:
     return 0
   return 0

def SaveExtractCommitInfo(fmt):
    try:
      process = subprocess.Popen(["git", "log", "-1",  "--pretty=%s" % fmt], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
      return process.communicate()[0].strip()
    except:
      return ""

buildVersion = parseBuildVersion(versionFileName) + 1

gen = dict()
gen["commit_message"] = SaveExtractCommitInfo("%s")
# + "\n\n" + SaveExtractCommitInfo("%b")).strip()
gen["commit_datetime"] = SaveExtractCommitInfo("%cd")
gen["commit_author_name"] = SaveExtractCommitInfo("%an")
gen["commiter_name"] = SaveExtractCommitInfo("%cn")
gen["build_datetime"] = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S (%Z)")
gen["commit_hash"] = SaveExtractCommitInfo("%H")
gen["build_version"] = str(buildVersion)

for name, value in gen.items():
   print("%s=\"%s\"" % (name, value)) 

with open(versionFileName,'w') as f:
   f.write("%s \"%d\"\n\n" % (buildVersionDef, buildVersion))
   for name, value in gen.items():
      f.write("\nchar const* %s() {\n" % name)
      f.write("    return R\"(%s)\";\n" % value)
      f.write("}\n") 

