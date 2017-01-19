app = angular.module('app', ['uiSlider']);                                        

//['$scope','$http', 
app.controller('ItemCtrl', ['$scope', function($scope){ 
  $scope.item = {                                                                 
    name: 'layerDump',                                                               
    cost: 0,                                                                    
    minAge: 25, 
    maxAge: 450 ,
    frame_index : 0,
    ratio : 5
  };                                                                              
  //$scope.currencyFormatting = function(value) { return value.toString() + " $"; };
  //$scope.currencyFormatting = function(value) { return value.toString() };
  
  $scope.radioLayer = {
    set: 0
    
    };
  
  
    //$http.get('dumpsys.json').success(function(data){
    //    $scope.layerObjects = data ;
    //});
    
    //dumpsrc.fetch().then(function(data) {
    //    $scope.data = data;
    //})


    $scope.curFrame = function() { return $scope.layerObjects[$scope.item.cost]} ;


     $scope.layerObjects = [  
       { "frameIdx" : "0", "name" : "com.android.systemui.ImageWallpaper"       , "frame" : [    0,    0, 1080, 1920], "type":"HWC"},
       { "frameIdx" : "0", "name" : "com.android.launcher3.Launcher"            , "frame" : [    0,    0, 1080, 1920], "type":"HWC"},
       { "frameIdx" : "0", "name" : "StatusBar"                                 , "frame" : [    0,    0, 1080,   75], "type":"HWC"},
       { "frameIdx" : "0", "name" : "NavigationBar"                             , "frame" : [    0, 1776, 1080, 1920], "type":"HWC"},
       { "frameIdx" : "0", "name" : "HWC_FRAMEBUFFER_TARGET"                    , "frame" : [    0,    0, 1080, 1920], "type":"FB TARGET"},
                                                                                
       { "frameIdx" : "1", "name" : "c_om.android.systemui.ImageWallpaper"      , "frame" : [    0,    0, 1080, 1920], "type":"GLES"},
       { "frameIdx" : "1", "name" : "c_om.android.launcher3.Launcher"           , "frame" : [    0,    0, 1080, 1920], "type":"GLES"},
       { "frameIdx" : "1", "name" : "N_avigationBar"                            , "frame" : [    0, 1776, 1080, 1920], "type":"GLES"},
       { "frameIdx" : "1", "name" : "H_WC_FRAMEBUFFER_TARGET"                   , "frame" : [    0,    0, 1080, 1920], "type":"FB TARGET"},
                                                                                
       { "frameIdx" : "2", "name" : "c_2_om.android.systemui.ImageWallpaper"    , "frame" : [    0,    0, 1080, 1920], "type":"HWC"},
       { "frameIdx" : "2", "name" : "c_2_om.android.launcher3.Launcher"         , "frame" : [    0,    0, 1080, 1920], "type":"HWC"},
       { "frameIdx" : "2", "name" : "S_2_tatusBar"                              , "frame" : [    0,    0, 1080,   75], "type":"GLES"},
       { "frameIdx" : "2", "name" : "H_2_WC_FRAMEBUFFER_TARGET"                 , "frame" : [    0,    0, 1080, 1920], "type":"FB TARGET"}, 
                                                                                            
       { "frameIdx" : "3", "name" : "cXX_3_om.android.systemui.ImageWallpaper"  , "frame" : [    0,    0, 1080, 1920], "type":"HWC"},
       { "frameIdx" : "3", "name" : "cXX_3_om.android.launcher3.Launcher"       , "frame" : [    0,    0, 1080, 1920], "type":"HWC"},
       { "frameIdx" : "3", "name" : "SXX_3_tatusBar"                            , "frame" : [    0,    0, 1080,   75], "type":"GLES"},
       { "frameIdx" : "3", "name" : "NXX_3_avigationBar"                        , "frame" : [    0, 1776, 1080, 1920], "type":"GLES"},
       { "frameIdx" : "3", "name" : "HXX_3_WC_FRAMEBUFFER_TARGET"               , "frame" : [    0,    0, 1080, 1920], "type":"FB TARGET"},
                                                                                            
       { "frameIdx" : "4", "name" : "cXXMM_4_om.android.systemui.ImageWallpaper", "frame" : [    0,    0, 1080, 1920], "type":"HWC"},
       { "frameIdx" : "4", "name" : "cXXMM_4_om.android.launcher3.Launcher"     , "frame" : [    0,    0, 1080, 1920], "type":"HWC"},
       { "frameIdx" : "4", "name" : "SXXMM_4_tatusBar"                          , "frame" : [    0,    0, 1080,   75], "type":"HWC"},
       { "frameIdx" : "4", "name" : "NXXMM_4_avigationBar"                      , "frame" : [    0, 1776, 1080, 1920], "type":"GLES"},
       { "frameIdx" : "4", "name" : "HXXMM_4_WC_FRAMEBUFFER_TARGET"             , "frame" : [    0,    0, 1080, 1920], "type":"FB TARGET"},
                                                                                            
        { "frameIdx" : "5", "name" : "cJJK_5_om.android.systemui.ImageWallpaper", "frame" : [    0,    0, 1080, 1920], "type":"HWC"},
        { "frameIdx" : "5", "name" : "cJJK_5_om.android.launcher3.Launcher"     , "frame" : [    0,    0, 1080, 1920], "type":"GLES"},
        { "frameIdx" : "5", "name" : "SJJK_5_tatusBar"                          , "frame" : [    0,    0, 1080,   75], "type":"GLES"},
        { "frameIdx" : "5", "name" : "NJJK_5_avigationBar"                      , "frame" : [    0, 1776, 1080, 1920], "type":"GLES"},
        { "frameIdx" : "5", "name" : "HJJK_5_WC_FRAMEBUFFER_TARGET"             , "frame" : [    0,    0, 1080, 1920], "type":"FB TARGET"}
 ];     


  
}]);                                                                              


//app.factory('dumpsrc', function($q, $timeout, $http) {
//    var mysrc = {
//        fetch: function(callback) {
//            
//            var deferred = $q.defer();
//
//            $timeout(function() {
//                $http.get('dumpsys.json').success(function(data) {
//                    deferred.resolve(data);
//                });
//            }, 30);
//
//            return deferred.promise;
//        }
//    };
//
//    return mysrc;
//});


                                                                                  