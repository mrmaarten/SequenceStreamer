# Create bin/data/ofxbraitsch directory if it doesn't exist
mkdir -p "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/ofxbraitsch/fonts"
mkdir -p "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/ofxbraitsch/icons"

# Copy the required resources from the addon's directory
cp -R "$OF_PATH/addons/ofxDatGui/bin/data/ofxbraitsch/fonts/"* "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/ofxbraitsch/fonts/"
cp -R "$OF_PATH/addons/ofxDatGui/bin/data/ofxbraitsch/icons/"* "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources/data/ofxbraitsch/icons/"