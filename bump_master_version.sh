#!/bin/bash

# This script needs to be edited to bump old develop version to new master version for new release.
# - run this script in develop branch 
# - after running this script merge develop into master
# - after running this script and merging develop into master, run bump_develop_version.sh in master and
#   merge master into develop

OLD_ROCBLAS_VERSION="15.3.0"
NEW_ROCBLAS_VERSION="14.3.1"

OLD_TENSILE_VERSION="tensile_tag \"develop\""
NEW_TENSILE_VERSION="tensile_tag 9151458d09e6e3715af016c4a0d9a12b845dc74b"

OLD_MINIMUM_REQUIRED_VERSION="MinimumRequiredVersion: 4.6.0"
NEW_MINIMUM_REQUIRED_VERSION="MinimumRequiredVersion: 4.6.0"

sed -i "s/${OLD_ROCBLAS_VERSION}/${NEW_ROCBLAS_VERSION}/g" CMakeLists.txt
sed -i "s/${OLD_TENSILE_VERSION}/${NEW_TENSILE_VERSION}/g" CMakeLists.txt

#only update yaml files for a Tensile major version change
#for FILE in library/src/blas3/Tensile/Logic/*/*yaml
#do
#  sed -i "s/${OLD_MINIMUM_REQUIRED_VERSION}/${NEW_MINIMUM_REQUIRED_VERSION}/" $FILE
#done
