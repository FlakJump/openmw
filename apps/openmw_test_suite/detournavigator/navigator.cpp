#include "operators.hpp"

#include <components/detournavigator/navigator.hpp>
#include <components/detournavigator/exceptions.hpp>

#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btCompoundShape.h>

#include <gtest/gtest.h>

#include <iterator>
#include <deque>

namespace
{
    using namespace testing;
    using namespace DetourNavigator;

    struct DetourNavigatorNavigatorTest : Test
    {
        Settings mSettings;
        std::unique_ptr<Navigator> mNavigator;
        osg::Vec3f mPlayerPosition;
        osg::Vec3f mAgentHalfExtents;
        osg::Vec3f mStart;
        osg::Vec3f mEnd;
        std::deque<osg::Vec3f> mPath;
        std::back_insert_iterator<std::deque<osg::Vec3f>> mOut;

        DetourNavigatorNavigatorTest()
            : mPlayerPosition(0, 0, 0)
            , mAgentHalfExtents(29, 29, 66)
            , mStart(-215, 215, 1)
            , mEnd(215, -215, 1)
            , mOut(mPath)
        {
            mSettings.mEnableWriteRecastMeshToFile = false;
            mSettings.mEnableWriteNavMeshToFile = false;
            mSettings.mEnableRecastMeshFileNameRevision = false;
            mSettings.mEnableNavMeshFileNameRevision = false;
            mSettings.mBorderSize = 16;
            mSettings.mCellHeight = 0.2f;
            mSettings.mCellSize = 0.2f;
            mSettings.mDetailSampleDist = 6;
            mSettings.mDetailSampleMaxError = 1;
            mSettings.mMaxClimb = 34;
            mSettings.mMaxSimplificationError = 1.3f;
            mSettings.mMaxSlope = 49;
            mSettings.mRecastScaleFactor = 0.017647058823529415f;
            mSettings.mSwimHeightScale = 0.89999997615814208984375f;
            mSettings.mMaxEdgeLen = 12;
            mSettings.mMaxNavMeshQueryNodes = 2048;
            mSettings.mMaxVertsPerPoly = 6;
            mSettings.mRegionMergeSize = 20;
            mSettings.mRegionMinSize = 8;
            mSettings.mTileSize = 64;
            mSettings.mAsyncNavMeshUpdaterThreads = 1;
            mSettings.mMaxNavMeshTilesCacheSize = 1024 * 1024;
            mSettings.mMaxPolygonPathSize = 1024;
            mSettings.mMaxSmoothPathSize = 1024;
            mSettings.mTrianglesPerChunk = 256;
            mSettings.mMaxPolys = 4096;
            mNavigator.reset(new Navigator(mSettings));
        }
    };

    TEST_F(DetourNavigatorNavigatorTest, find_path_for_empty_should_throw_exception)
    {
        EXPECT_THROW(mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_walk, mOut), InvalidArgument);
    }

    TEST_F(DetourNavigatorNavigatorTest, find_path_for_existing_agent_with_no_navmesh_should_throw_exception)
    {
        mNavigator->addAgent(mAgentHalfExtents);
        EXPECT_THROW(mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_walk, mOut), NavigatorException);
    }

    TEST_F(DetourNavigatorNavigatorTest, find_path_for_removed_agent_should_throw_exception)
    {
        mNavigator->addAgent(mAgentHalfExtents);
        mNavigator->removeAgent(mAgentHalfExtents);
        EXPECT_THROW(mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_walk, mOut), InvalidArgument);
    }

    TEST_F(DetourNavigatorNavigatorTest, add_agent_should_count_each_agent)
    {
        mNavigator->addAgent(mAgentHalfExtents);
        mNavigator->addAgent(mAgentHalfExtents);
        mNavigator->removeAgent(mAgentHalfExtents);
        EXPECT_THROW(mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_walk, mOut), NavigatorException);
    }

    TEST_F(DetourNavigatorNavigatorTest, update_then_find_path_should_return_path)
    {
        const std::array<btScalar, 5 * 5> heightfieldData {{
            0,   0,    0,    0,    0,
            0, -25,  -25,  -25,  -25,
            0, -25, -100, -100, -100,
            0, -25, -100, -100, -100,
            0, -25, -100, -100, -100,
        }};
        btHeightfieldTerrainShape shape(5, 5, heightfieldData.data(), 1, 0, 0, 2, PHY_FLOAT, false);
        shape.setLocalScaling(btVector3(128, 128, 1));

        mNavigator->addAgent(mAgentHalfExtents);
        mNavigator->addObject(ObjectId(&shape), shape, btTransform::getIdentity());
        mNavigator->update(mPlayerPosition);
        mNavigator->wait();

        mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_walk, mOut);

        EXPECT_EQ(mPath, std::deque<osg::Vec3f>({
            osg::Vec3f(-215, 215, 1.85963428020477294921875),
            osg::Vec3f(-194.9653167724609375, 194.9653167724609375, -6.5760211944580078125),
            osg::Vec3f(-174.930633544921875, 174.930633544921875, -15.01167774200439453125),
            osg::Vec3f(-154.8959503173828125, 154.8959503173828125, -23.4473323822021484375),
            osg::Vec3f(-134.86126708984375, 134.86126708984375, -31.8829898834228515625),
            osg::Vec3f(-114.82657623291015625, 114.82657623291015625, -40.3186492919921875),
            osg::Vec3f(-94.7918853759765625, 94.7918853759765625, -47.39907073974609375),
            osg::Vec3f(-74.75719451904296875, 74.75719451904296875, -53.7258148193359375),
            osg::Vec3f(-54.722499847412109375, 54.722499847412109375, -60.052555084228515625),
            osg::Vec3f(-34.68780517578125, 34.68780517578125, -66.37929534912109375),
            osg::Vec3f(-14.6531162261962890625, 14.6531162261962890625, -72.70604705810546875),
            osg::Vec3f(5.3815765380859375, -5.3815765380859375, -75.35065460205078125),
            osg::Vec3f(25.41626739501953125, -25.41626739501953125, -67.96945953369140625),
            osg::Vec3f(45.450958251953125, -45.450958251953125, -60.58824920654296875),
            osg::Vec3f(65.48564910888671875, -65.48564910888671875, -53.20705413818359375),
            osg::Vec3f(85.5203399658203125, -85.5203399658203125, -45.825855255126953125),
            osg::Vec3f(105.55503082275390625, -105.55503082275390625, -38.44464874267578125),
            osg::Vec3f(125.5897216796875, -125.5897216796875, -31.063449859619140625),
            osg::Vec3f(145.6244049072265625, -145.6244049072265625, -23.6822509765625),
            osg::Vec3f(165.659088134765625, -165.659088134765625, -16.3010540008544921875),
            osg::Vec3f(185.6937713623046875, -185.6937713623046875, -8.91985416412353515625),
            osg::Vec3f(205.7284698486328125, -205.7284698486328125, -1.53864824771881103515625),
            osg::Vec3f(215, -215, 1.877177715301513671875),
        })) << mPath;
    }

    TEST_F(DetourNavigatorNavigatorTest, add_object_should_change_navmesh)
    {
        const std::array<btScalar, 5 * 5> heightfieldData {{
            0,   0,    0,    0,    0,
            0, -25,  -25,  -25,  -25,
            0, -25, -100, -100, -100,
            0, -25, -100, -100, -100,
            0, -25, -100, -100, -100,
        }};
        btHeightfieldTerrainShape heightfieldShape(5, 5, heightfieldData.data(), 1, 0, 0, 2, PHY_FLOAT, false);
        heightfieldShape.setLocalScaling(btVector3(128, 128, 1));

        btBoxShape boxShape(btVector3(20, 20, 100));
        btCompoundShape compoundShape;
        compoundShape.addChildShape(btTransform(btMatrix3x3::getIdentity(), btVector3(0, 0, 0)), &boxShape);

        mNavigator->addAgent(mAgentHalfExtents);
        mNavigator->addObject(ObjectId(&heightfieldShape), heightfieldShape, btTransform::getIdentity());
        mNavigator->update(mPlayerPosition);
        mNavigator->wait();

        mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_walk, std::back_inserter(mPath));

        EXPECT_EQ(mPath, std::deque<osg::Vec3f>({
            osg::Vec3f(-215, 215, 1.85963428020477294921875),
            osg::Vec3f(-194.9653167724609375, 194.9653167724609375, -6.5760211944580078125),
            osg::Vec3f(-174.930633544921875, 174.930633544921875, -15.01167774200439453125),
            osg::Vec3f(-154.8959503173828125, 154.8959503173828125, -23.4473323822021484375),
            osg::Vec3f(-134.86126708984375, 134.86126708984375, -31.8829898834228515625),
            osg::Vec3f(-114.82657623291015625, 114.82657623291015625, -40.3186492919921875),
            osg::Vec3f(-94.7918853759765625, 94.7918853759765625, -47.39907073974609375),
            osg::Vec3f(-74.75719451904296875, 74.75719451904296875, -53.7258148193359375),
            osg::Vec3f(-54.722499847412109375, 54.722499847412109375, -60.052555084228515625),
            osg::Vec3f(-34.68780517578125, 34.68780517578125, -66.37929534912109375),
            osg::Vec3f(-14.6531162261962890625, 14.6531162261962890625, -72.70604705810546875),
            osg::Vec3f(5.3815765380859375, -5.3815765380859375, -75.35065460205078125),
            osg::Vec3f(25.41626739501953125, -25.41626739501953125, -67.96945953369140625),
            osg::Vec3f(45.450958251953125, -45.450958251953125, -60.58824920654296875),
            osg::Vec3f(65.48564910888671875, -65.48564910888671875, -53.20705413818359375),
            osg::Vec3f(85.5203399658203125, -85.5203399658203125, -45.825855255126953125),
            osg::Vec3f(105.55503082275390625, -105.55503082275390625, -38.44464874267578125),
            osg::Vec3f(125.5897216796875, -125.5897216796875, -31.063449859619140625),
            osg::Vec3f(145.6244049072265625, -145.6244049072265625, -23.6822509765625),
            osg::Vec3f(165.659088134765625, -165.659088134765625, -16.3010540008544921875),
            osg::Vec3f(185.6937713623046875, -185.6937713623046875, -8.91985416412353515625),
            osg::Vec3f(205.7284698486328125, -205.7284698486328125, -1.53864824771881103515625),
            osg::Vec3f(215, -215, 1.877177715301513671875),
        })) << mPath;

        mNavigator->addObject(ObjectId(&compoundShape), compoundShape, btTransform::getIdentity());
        mNavigator->update(mPlayerPosition);
        mNavigator->wait();

        mPath.clear();
        mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_walk, std::back_inserter(mPath));

        EXPECT_EQ(mPath, std::deque<osg::Vec3f>({
            osg::Vec3f(-215, 215, 1.87827122211456298828125),
            osg::Vec3f(-199.7968292236328125, 191.09100341796875, -3.54876613616943359375),
            osg::Vec3f(-184.5936431884765625, 167.1819915771484375, -8.97847270965576171875),
            osg::Vec3f(-169.3904571533203125, 143.2729949951171875, -14.40817737579345703125),
            osg::Vec3f(-154.1872711181640625, 119.36397552490234375, -19.837890625),
            osg::Vec3f(-138.9840850830078125, 95.45496368408203125, -25.2675952911376953125),
            osg::Vec3f(-123.78090667724609375, 71.54595184326171875, -30.6972980499267578125),
            osg::Vec3f(-108.57772064208984375, 47.636936187744140625, -36.12701416015625),
            osg::Vec3f(-93.3745269775390625, 23.7279262542724609375, -40.754688262939453125),
            osg::Vec3f(-78.17134857177734375, -0.18108306825160980224609375, -37.128787994384765625),
            osg::Vec3f(-62.968158721923828125, -24.0900936126708984375, -33.50289154052734375),
            osg::Vec3f(-47.764972686767578125, -47.999103546142578125, -30.797946929931640625),
            osg::Vec3f(-23.852447509765625, -63.196765899658203125, -33.97112274169921875),
            osg::Vec3f(0.0600789971649646759033203125, -78.39443206787109375, -37.14543914794921875),
            osg::Vec3f(23.97260284423828125, -93.5920867919921875, -40.7740936279296875),
            osg::Vec3f(47.885128021240234375, -108.78974151611328125, -36.051288604736328125),
            osg::Vec3f(71.7976531982421875, -123.98740386962890625, -30.62355804443359375),
            osg::Vec3f(95.71018218994140625, -139.18505859375, -25.1958160400390625),
            osg::Vec3f(119.6226959228515625, -154.382720947265625, -19.7680912017822265625),
            osg::Vec3f(143.53521728515625, -169.58038330078125, -14.3403491973876953125),
            osg::Vec3f(167.4477386474609375, -184.778045654296875, -8.91261768341064453125),
            osg::Vec3f(191.360260009765625, -199.9757080078125, -3.484879016876220703125),
            osg::Vec3f(215, -215, 1.87827455997467041015625),
        })) << mPath;
    }

    TEST_F(DetourNavigatorNavigatorTest, update_changed_object_should_change_navmesh)
    {
        const std::array<btScalar, 5 * 5> heightfieldData {{
            0,   0,    0,    0,    0,
            0, -25,  -25,  -25,  -25,
            0, -25, -100, -100, -100,
            0, -25, -100, -100, -100,
            0, -25, -100, -100, -100,
        }};
        btHeightfieldTerrainShape heightfieldShape(5, 5, heightfieldData.data(), 1, 0, 0, 2, PHY_FLOAT, false);
        heightfieldShape.setLocalScaling(btVector3(128, 128, 1));

        btBoxShape boxShape(btVector3(20, 20, 100));
        btCompoundShape compoundShape;
        compoundShape.addChildShape(btTransform(btMatrix3x3::getIdentity(), btVector3(0, 0, 0)), &boxShape);

        mNavigator->addAgent(mAgentHalfExtents);
        mNavigator->addObject(ObjectId(&heightfieldShape), heightfieldShape, btTransform::getIdentity());
        mNavigator->addObject(ObjectId(&compoundShape), compoundShape, btTransform::getIdentity());
        mNavigator->update(mPlayerPosition);
        mNavigator->wait();

        mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_walk, std::back_inserter(mPath));

        EXPECT_EQ(mPath, std::deque<osg::Vec3f>({
            osg::Vec3f(-215, 215, 1.87827122211456298828125),
            osg::Vec3f(-199.7968292236328125, 191.09100341796875, -3.54876613616943359375),
            osg::Vec3f(-184.5936431884765625, 167.1819915771484375, -8.97847270965576171875),
            osg::Vec3f(-169.3904571533203125, 143.2729949951171875, -14.40817737579345703125),
            osg::Vec3f(-154.1872711181640625, 119.36397552490234375, -19.837890625),
            osg::Vec3f(-138.9840850830078125, 95.45496368408203125, -25.2675952911376953125),
            osg::Vec3f(-123.78090667724609375, 71.54595184326171875, -30.6972980499267578125),
            osg::Vec3f(-108.57772064208984375, 47.636936187744140625, -36.12701416015625),
            osg::Vec3f(-93.3745269775390625, 23.7279262542724609375, -40.754688262939453125),
            osg::Vec3f(-78.17134857177734375, -0.18108306825160980224609375, -37.128787994384765625),
            osg::Vec3f(-62.968158721923828125, -24.0900936126708984375, -33.50289154052734375),
            osg::Vec3f(-47.764972686767578125, -47.999103546142578125, -30.797946929931640625),
            osg::Vec3f(-23.852447509765625, -63.196765899658203125, -33.97112274169921875),
            osg::Vec3f(0.0600789971649646759033203125, -78.39443206787109375, -37.14543914794921875),
            osg::Vec3f(23.97260284423828125, -93.5920867919921875, -40.7740936279296875),
            osg::Vec3f(47.885128021240234375, -108.78974151611328125, -36.051288604736328125),
            osg::Vec3f(71.7976531982421875, -123.98740386962890625, -30.62355804443359375),
            osg::Vec3f(95.71018218994140625, -139.18505859375, -25.1958160400390625),
            osg::Vec3f(119.6226959228515625, -154.382720947265625, -19.7680912017822265625),
            osg::Vec3f(143.53521728515625, -169.58038330078125, -14.3403491973876953125),
            osg::Vec3f(167.4477386474609375, -184.778045654296875, -8.91261768341064453125),
            osg::Vec3f(191.360260009765625, -199.9757080078125, -3.484879016876220703125),
            osg::Vec3f(215, -215, 1.87827455997467041015625),
        })) << mPath;

        compoundShape.updateChildTransform(0, btTransform(btMatrix3x3::getIdentity(), btVector3(1000, 0, 0)));

        mNavigator->updateObject(ObjectId(&compoundShape), compoundShape, btTransform::getIdentity());
        mNavigator->update(mPlayerPosition);
        mNavigator->wait();

        mPath.clear();
        mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_walk, mOut);

        EXPECT_EQ(mPath, std::deque<osg::Vec3f>({
            osg::Vec3f(-215, 215, 1.85963428020477294921875),
            osg::Vec3f(-194.9653167724609375, 194.9653167724609375, -6.5760211944580078125),
            osg::Vec3f(-174.930633544921875, 174.930633544921875, -15.01167774200439453125),
            osg::Vec3f(-154.8959503173828125, 154.8959503173828125, -23.4473323822021484375),
            osg::Vec3f(-134.86126708984375, 134.86126708984375, -31.8829898834228515625),
            osg::Vec3f(-114.82657623291015625, 114.82657623291015625, -40.3186492919921875),
            osg::Vec3f(-94.7918853759765625, 94.7918853759765625, -47.39907073974609375),
            osg::Vec3f(-74.75719451904296875, 74.75719451904296875, -53.7258148193359375),
            osg::Vec3f(-54.722499847412109375, 54.722499847412109375, -60.052555084228515625),
            osg::Vec3f(-34.68780517578125, 34.68780517578125, -66.37929534912109375),
            osg::Vec3f(-14.6531162261962890625, 14.6531162261962890625, -72.70604705810546875),
            osg::Vec3f(5.3815765380859375, -5.3815765380859375, -75.35065460205078125),
            osg::Vec3f(25.41626739501953125, -25.41626739501953125, -67.96945953369140625),
            osg::Vec3f(45.450958251953125, -45.450958251953125, -60.58824920654296875),
            osg::Vec3f(65.48564910888671875, -65.48564910888671875, -53.20705413818359375),
            osg::Vec3f(85.5203399658203125, -85.5203399658203125, -45.825855255126953125),
            osg::Vec3f(105.55503082275390625, -105.55503082275390625, -38.44464874267578125),
            osg::Vec3f(125.5897216796875, -125.5897216796875, -31.063449859619140625),
            osg::Vec3f(145.6244049072265625, -145.6244049072265625, -23.6822509765625),
            osg::Vec3f(165.659088134765625, -165.659088134765625, -16.3010540008544921875),
            osg::Vec3f(185.6937713623046875, -185.6937713623046875, -8.91985416412353515625),
            osg::Vec3f(205.7284698486328125, -205.7284698486328125, -1.53864824771881103515625),
            osg::Vec3f(215, -215, 1.877177715301513671875),
        })) << mPath;
    }

    TEST_F(DetourNavigatorNavigatorTest, for_overlapping_heightfields_should_use_higher)
    {
        const std::array<btScalar, 5 * 5> heightfieldData {{
            0,   0,    0,    0,    0,
            0, -25,  -25,  -25,  -25,
            0, -25, -100, -100, -100,
            0, -25, -100, -100, -100,
            0, -25, -100, -100, -100,
        }};
        btHeightfieldTerrainShape shape(5, 5, heightfieldData.data(), 1, 0, 0, 2, PHY_FLOAT, false);
        shape.setLocalScaling(btVector3(128, 128, 1));

        const std::array<btScalar, 5 * 5> heightfieldData2 {{
            -25, -25, -25, -25, -25,
            -25, -25, -25, -25, -25,
            -25, -25, -25, -25, -25,
            -25, -25, -25, -25, -25,
            -25, -25, -25, -25, -25,
        }};
        btHeightfieldTerrainShape shape2(5, 5, heightfieldData2.data(), 1, 0, 0, 2, PHY_FLOAT, false);
        shape2.setLocalScaling(btVector3(128, 128, 1));

        mNavigator->addAgent(mAgentHalfExtents);
        mNavigator->addObject(ObjectId(&shape), shape, btTransform::getIdentity());
        mNavigator->addObject(ObjectId(&shape2), shape2, btTransform::getIdentity());
        mNavigator->update(mPlayerPosition);
        mNavigator->wait();

        mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_walk, mOut);

        EXPECT_EQ(mPath, std::deque<osg::Vec3f>({
            osg::Vec3f(-215, 215, 1.96328866481781005859375),
            osg::Vec3f(-194.9653167724609375, 194.9653167724609375, -0.2422157227993011474609375),
            osg::Vec3f(-174.930633544921875, 174.930633544921875, -2.44772052764892578125),
            osg::Vec3f(-154.8959503173828125, 154.8959503173828125, -4.653223514556884765625),
            osg::Vec3f(-134.86126708984375, 134.86126708984375, -6.858728885650634765625),
            osg::Vec3f(-114.82657623291015625, 114.82657623291015625, -9.0642337799072265625),
            osg::Vec3f(-94.7918853759765625, 94.7918853759765625, -11.26973724365234375),
            osg::Vec3f(-74.75719451904296875, 74.75719451904296875, -13.26497173309326171875),
            osg::Vec3f(-54.722499847412109375, 54.722499847412109375, -15.24860286712646484375),
            osg::Vec3f(-34.68780517578125, 34.68780517578125, -17.2322368621826171875),
            osg::Vec3f(-14.6531162261962890625, 14.6531162261962890625, -19.2158660888671875),
            osg::Vec3f(5.3815765380859375, -5.3815765380859375, -20.1338443756103515625),
            osg::Vec3f(25.41626739501953125, -25.41626739501953125, -18.150211334228515625),
            osg::Vec3f(45.450958251953125, -45.450958251953125, -16.1665802001953125),
            osg::Vec3f(65.48564910888671875, -65.48564910888671875, -14.18294811248779296875),
            osg::Vec3f(85.5203399658203125, -85.5203399658203125, -12.19931507110595703125),
            osg::Vec3f(105.55503082275390625, -105.55503082275390625, -10.08488559722900390625),
            osg::Vec3f(125.5897216796875, -125.5897216796875, -7.879383563995361328125),
            osg::Vec3f(145.6244049072265625, -145.6244049072265625, -5.673877239227294921875),
            osg::Vec3f(165.659088134765625, -165.659088134765625, -3.4683735370635986328125),
            osg::Vec3f(185.6937713623046875, -185.6937713623046875, -1.2628715038299560546875),
            osg::Vec3f(205.7284698486328125, -205.7284698486328125, 0.9426348209381103515625),
            osg::Vec3f(215, -215, 1.96328866481781005859375),
        })) << mPath;
    }

    TEST_F(DetourNavigatorNavigatorTest, path_should_be_around_avoid_shape)
    {
        std::array<btScalar, 5 * 5> heightfieldData {{
            0,   0,    0,    0,    0,
            0, -25,  -25,  -25,  -25,
            0, -25, -100, -100, -100,
            0, -25, -100, -100, -100,
            0, -25, -100, -100, -100,
        }};
        btHeightfieldTerrainShape shape(5, 5, heightfieldData.data(), 1, 0, 0, 2, PHY_FLOAT, false);
        shape.setLocalScaling(btVector3(128, 128, 1));

        std::array<btScalar, 5 * 5> heightfieldDataAvoid {{
            -25, -25, -25, -25, -25,
            -25, -25, -25, -25, -25,
            -25, -25, -25, -25, -25,
            -25, -25, -25, -25, -25,
            -25, -25, -25, -25, -25,
        }};
        btHeightfieldTerrainShape shapeAvoid(5, 5, heightfieldDataAvoid.data(), 1, 0, 0, 2, PHY_FLOAT, false);
        shapeAvoid.setLocalScaling(btVector3(128, 128, 1));

        mNavigator->addAgent(mAgentHalfExtents);
        mNavigator->addObject(ObjectId(&shape), ObjectShapes {shape, &shapeAvoid}, btTransform::getIdentity());
        mNavigator->update(mPlayerPosition);
        mNavigator->wait();

        mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_walk, mOut);

        EXPECT_EQ(mPath, std::deque<osg::Vec3f>({
            osg::Vec3f(-215, 215, 1.9393787384033203125),
            osg::Vec3f(-200.8159637451171875, 190.47265625, -0.639537751674652099609375),
            osg::Vec3f(-186.6319427490234375, 165.9453125, -3.2184507846832275390625),
            osg::Vec3f(-172.447906494140625, 141.41796875, -5.797363758087158203125),
            osg::Vec3f(-158.263885498046875, 116.8906097412109375, -8.37627887725830078125),
            osg::Vec3f(-144.079864501953125, 92.3632659912109375, -10.95519161224365234375),
            osg::Vec3f(-129.89581298828125, 67.83591461181640625, -13.534107208251953125),
            osg::Vec3f(-115.7117919921875, 43.308563232421875, -16.1130199432373046875),
            osg::Vec3f(-101.5277557373046875, 18.7812137603759765625, -18.6919345855712890625),
            osg::Vec3f(-87.34372711181640625, -5.7461376190185546875, -20.4680538177490234375),
            osg::Vec3f(-67.02922821044921875, -25.4970550537109375, -20.514247894287109375),
            osg::Vec3f(-46.714717864990234375, -45.2479705810546875, -20.5604457855224609375),
            osg::Vec3f(-26.40021514892578125, -64.99889373779296875, -20.6066417694091796875),
            osg::Vec3f(-6.085712432861328125, -84.74980926513671875, -20.652835845947265625),
            osg::Vec3f(14.22879505157470703125, -104.50072479248046875, -18.151393890380859375),
            osg::Vec3f(39.05098724365234375, -118.16222381591796875, -15.6674861907958984375),
            osg::Vec3f(63.87317657470703125, -131.82373046875, -13.18357944488525390625),
            osg::Vec3f(88.69537353515625, -145.4852142333984375, -10.69967365264892578125),
            osg::Vec3f(113.51757049560546875, -159.146697998046875, -8.21576690673828125),
            osg::Vec3f(138.3397674560546875, -172.808197021484375, -5.731858730316162109375),
            osg::Vec3f(163.1619720458984375, -186.469696044921875, -3.2479503154754638671875),
            osg::Vec3f(187.984161376953125, -200.1311798095703125, -0.764044582843780517578125),
            osg::Vec3f(212.8063507080078125, -213.7926788330078125, 1.7198636531829833984375),
            osg::Vec3f(215, -215, 1.93937528133392333984375),
        })) << mPath;
    }

    TEST_F(DetourNavigatorNavigatorTest, path_should_be_over_water_ground_lower_than_water_with_only_swim_flag)
    {
        std::array<btScalar, 5 * 5> heightfieldData {{
            -50,  -50,  -50,  -50,    0,
            -50, -100, -150, -100,  -50,
            -50, -150, -200, -150, -100,
            -50, -100, -150, -100, -100,
              0,  -50, -100, -100, -100,
        }};
        btHeightfieldTerrainShape shape(5, 5, heightfieldData.data(), 1, 0, 0, 2, PHY_FLOAT, false);
        shape.setLocalScaling(btVector3(128, 128, 1));

        mNavigator->addAgent(mAgentHalfExtents);
        mNavigator->addWater(osg::Vec2i(0, 0), 128 * 4, 300, btTransform::getIdentity());
        mNavigator->addObject(ObjectId(&shape), shape, btTransform::getIdentity());
        mNavigator->update(mPlayerPosition);
        mNavigator->wait();

        mStart.x() = 0;
        mStart.z() = 300;
        mEnd.x() = 0;
        mEnd.z() = 300;

        mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_swim, mOut);

        EXPECT_EQ(mPath, std::deque<osg::Vec3f>({
            osg::Vec3f(0, 215, 185.33331298828125),
            osg::Vec3f(0, 186.6666717529296875, 185.33331298828125),
            osg::Vec3f(0, 158.333343505859375, 185.33331298828125),
            osg::Vec3f(0, 130.0000152587890625, 185.33331298828125),
            osg::Vec3f(0, 101.66667938232421875, 185.33331298828125),
            osg::Vec3f(0, 73.333343505859375, 185.33331298828125),
            osg::Vec3f(0, 45.0000152587890625, 185.33331298828125),
            osg::Vec3f(0, 16.6666812896728515625, 185.33331298828125),
            osg::Vec3f(0, -11.66664981842041015625, 185.33331298828125),
            osg::Vec3f(0, -39.999980926513671875, 185.33331298828125),
            osg::Vec3f(0, -68.33331298828125, 185.33331298828125),
            osg::Vec3f(0, -96.66664886474609375, 185.33331298828125),
            osg::Vec3f(0, -124.99997711181640625, 185.33331298828125),
            osg::Vec3f(0, -153.33331298828125, 185.33331298828125),
            osg::Vec3f(0, -181.6666412353515625, 185.33331298828125),
            osg::Vec3f(0, -209.999969482421875, 185.33331298828125),
            osg::Vec3f(0, -215, 185.33331298828125),
        })) << mPath;
    }

    TEST_F(DetourNavigatorNavigatorTest, path_should_be_over_water_when_ground_cross_water_with_swim_and_walk_flags)
    {
        std::array<btScalar, 7 * 7> heightfieldData {{
            0,    0,    0,    0,    0,    0, 0,
            0, -100, -100, -100, -100, -100, 0,
            0, -100, -150, -150, -150, -100, 0,
            0, -100, -150, -200, -150, -100, 0,
            0, -100, -150, -150, -150, -100, 0,
            0, -100, -100, -100, -100, -100, 0,
            0,    0,    0,    0,    0,    0, 0,
        }};
        btHeightfieldTerrainShape shape(7, 7, heightfieldData.data(), 1, 0, 0, 2, PHY_FLOAT, false);
        shape.setLocalScaling(btVector3(128, 128, 1));

        mNavigator->addAgent(mAgentHalfExtents);
        mNavigator->addWater(osg::Vec2i(0, 0), 128 * 4, -25, btTransform::getIdentity());
        mNavigator->addObject(ObjectId(&shape), shape, btTransform::getIdentity());
        mNavigator->update(mPlayerPosition);
        mNavigator->wait();

        mStart.x() = 0;
        mEnd.x() = 0;

        mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_swim | Flag_walk, mOut);

        EXPECT_EQ(mPath, std::deque<osg::Vec3f>({
            osg::Vec3f(0, 215, -94.75363922119140625),
            osg::Vec3f(0, 186.6666717529296875, -106.0000152587890625),
            osg::Vec3f(0, 158.333343505859375, -115.85507965087890625),
            osg::Vec3f(0, 130.0000152587890625, -125.71016693115234375),
            osg::Vec3f(0, 101.66667938232421875, -135.5652313232421875),
            osg::Vec3f(0, 73.333343505859375, -143.3333587646484375),
            osg::Vec3f(0, 45.0000152587890625, -143.3333587646484375),
            osg::Vec3f(0, 16.6666812896728515625, -143.3333587646484375),
            osg::Vec3f(0, -11.66664981842041015625, -143.3333587646484375),
            osg::Vec3f(0, -39.999980926513671875, -143.3333587646484375),
            osg::Vec3f(0, -68.33331298828125, -143.3333587646484375),
            osg::Vec3f(0, -96.66664886474609375, -137.3043670654296875),
            osg::Vec3f(0, -124.99997711181640625, -127.44930267333984375),
            osg::Vec3f(0, -153.33331298828125, -117.5942230224609375),
            osg::Vec3f(0, -181.6666412353515625, -107.7391510009765625),
            osg::Vec3f(0, -209.999969482421875, -97.79712677001953125),
            osg::Vec3f(0, -215, -94.753631591796875),
        })) << mPath;
    }

    TEST_F(DetourNavigatorNavigatorTest, path_should_be_over_water_when_ground_cross_water_with_max_int_cells_size_and_swim_and_walk_flags)
    {
        std::array<btScalar, 7 * 7> heightfieldData {{
            0,    0,    0,    0,    0,    0, 0,
            0, -100, -100, -100, -100, -100, 0,
            0, -100, -150, -150, -150, -100, 0,
            0, -100, -150, -200, -150, -100, 0,
            0, -100, -150, -150, -150, -100, 0,
            0, -100, -100, -100, -100, -100, 0,
            0,    0,    0,    0,    0,    0, 0,
        }};
        btHeightfieldTerrainShape shape(7, 7, heightfieldData.data(), 1, 0, 0, 2, PHY_FLOAT, false);
        shape.setLocalScaling(btVector3(128, 128, 1));

        mNavigator->addAgent(mAgentHalfExtents);
        mNavigator->addObject(ObjectId(&shape), shape, btTransform::getIdentity());
        mNavigator->addWater(osg::Vec2i(0, 0), std::numeric_limits<int>::max(), -25, btTransform::getIdentity());
        mNavigator->update(mPlayerPosition);
        mNavigator->wait();

        mStart.x() = 0;
        mEnd.x() = 0;

        mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_swim | Flag_walk, mOut);

        EXPECT_EQ(mPath, std::deque<osg::Vec3f>({
            osg::Vec3f(0, 215, -94.75363922119140625),
            osg::Vec3f(0, 186.6666717529296875, -106.0000152587890625),
            osg::Vec3f(0, 158.333343505859375, -115.85507965087890625),
            osg::Vec3f(0, 130.0000152587890625, -125.71016693115234375),
            osg::Vec3f(0, 101.66667938232421875, -135.5652313232421875),
            osg::Vec3f(0, 73.333343505859375, -143.3333587646484375),
            osg::Vec3f(0, 45.0000152587890625, -143.3333587646484375),
            osg::Vec3f(0, 16.6666812896728515625, -143.3333587646484375),
            osg::Vec3f(0, -11.66664981842041015625, -143.3333587646484375),
            osg::Vec3f(0, -39.999980926513671875, -143.3333587646484375),
            osg::Vec3f(0, -68.33331298828125, -143.3333587646484375),
            osg::Vec3f(0, -96.66664886474609375, -137.3043670654296875),
            osg::Vec3f(0, -124.99997711181640625, -127.44930267333984375),
            osg::Vec3f(0, -153.33331298828125, -117.5942230224609375),
            osg::Vec3f(0, -181.6666412353515625, -107.7391510009765625),
            osg::Vec3f(0, -209.999969482421875, -97.79712677001953125),
            osg::Vec3f(0, -215, -94.753631591796875),
        })) << mPath;
    }

    TEST_F(DetourNavigatorNavigatorTest, path_should_be_over_ground_when_ground_cross_water_with_only_walk_flag)
    {
        std::array<btScalar, 7 * 7> heightfieldData {{
            0,    0,    0,    0,    0,    0, 0,
            0, -100, -100, -100, -100, -100, 0,
            0, -100, -150, -150, -150, -100, 0,
            0, -100, -150, -200, -150, -100, 0,
            0, -100, -150, -150, -150, -100, 0,
            0, -100, -100, -100, -100, -100, 0,
            0,    0,    0,    0,    0,    0, 0,
        }};
        btHeightfieldTerrainShape shape(7, 7, heightfieldData.data(), 1, 0, 0, 2, PHY_FLOAT, false);
        shape.setLocalScaling(btVector3(128, 128, 1));

        mNavigator->addAgent(mAgentHalfExtents);
        mNavigator->addWater(osg::Vec2i(0, 0), 128 * 4, -25, btTransform::getIdentity());
        mNavigator->addObject(ObjectId(&shape), shape, btTransform::getIdentity());
        mNavigator->update(mPlayerPosition);
        mNavigator->wait();

        mStart.x() = 0;
        mEnd.x() = 0;

        mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_walk, mOut);

        EXPECT_EQ(mPath, std::deque<osg::Vec3f>({
            osg::Vec3f(0, 215, -94.75363922119140625),
            osg::Vec3f(9.8083515167236328125, 188.4185333251953125, -105.19994354248046875),
            osg::Vec3f(19.6167049407958984375, 161.837066650390625, -114.25496673583984375),
            osg::Vec3f(29.42505645751953125, 135.255615234375, -123.309967041015625),
            osg::Vec3f(39.23340606689453125, 108.674163818359375, -132.3649749755859375),
            osg::Vec3f(49.04175567626953125, 82.09270477294921875, -137.2874755859375),
            osg::Vec3f(58.8501129150390625, 55.5112457275390625, -139.2451171875),
            osg::Vec3f(68.6584625244140625, 28.9297885894775390625, -141.2027740478515625),
            osg::Vec3f(78.4668121337890625, 2.3483295440673828125, -143.1604156494140625),
            osg::Vec3f(88.27516937255859375, -24.233127593994140625, -141.3894805908203125),
            osg::Vec3f(83.73651885986328125, -52.2005767822265625, -142.3761444091796875),
            osg::Vec3f(79.19786834716796875, -80.16802978515625, -143.114837646484375),
            osg::Vec3f(64.8477935791015625, -104.598602294921875, -137.840911865234375),
            osg::Vec3f(50.497714996337890625, -129.0291748046875, -131.45831298828125),
            osg::Vec3f(36.147632598876953125, -153.459747314453125, -121.42321014404296875),
            osg::Vec3f(21.7975559234619140625, -177.8903350830078125, -111.38809967041015625),
            osg::Vec3f(7.44747829437255859375, -202.3209075927734375, -101.1938323974609375),
            osg::Vec3f(0, -215, -94.753631591796875),
        })) << mPath;
    }

    TEST_F(DetourNavigatorNavigatorTest, update_remove_and_update_then_find_path_should_return_path)
    {
        const std::array<btScalar, 5 * 5> heightfieldData {{
            0,   0,    0,    0,    0,
            0, -25,  -25,  -25,  -25,
            0, -25, -100, -100, -100,
            0, -25, -100, -100, -100,
            0, -25, -100, -100, -100,
        }};
        btHeightfieldTerrainShape shape(5, 5, heightfieldData.data(), 1, 0, 0, 2, PHY_FLOAT, false);
        shape.setLocalScaling(btVector3(128, 128, 1));

        mNavigator->addAgent(mAgentHalfExtents);
        mNavigator->addObject(ObjectId(&shape), shape, btTransform::getIdentity());
        mNavigator->update(mPlayerPosition);
        mNavigator->wait();

        mNavigator->removeObject(ObjectId(&shape));
        mNavigator->update(mPlayerPosition);
        mNavigator->wait();

        mNavigator->addObject(ObjectId(&shape), shape, btTransform::getIdentity());
        mNavigator->update(mPlayerPosition);
        mNavigator->wait();

        mNavigator->findPath(mAgentHalfExtents, mStart, mEnd, Flag_walk, mOut);

        EXPECT_EQ(mPath, std::deque<osg::Vec3f>({
            osg::Vec3f(-215, 215, 1.85963428020477294921875),
            osg::Vec3f(-194.9653167724609375, 194.9653167724609375, -6.5760211944580078125),
            osg::Vec3f(-174.930633544921875, 174.930633544921875, -15.01167774200439453125),
            osg::Vec3f(-154.8959503173828125, 154.8959503173828125, -23.4473323822021484375),
            osg::Vec3f(-134.86126708984375, 134.86126708984375, -31.8829898834228515625),
            osg::Vec3f(-114.82657623291015625, 114.82657623291015625, -40.3186492919921875),
            osg::Vec3f(-94.7918853759765625, 94.7918853759765625, -47.39907073974609375),
            osg::Vec3f(-74.75719451904296875, 74.75719451904296875, -53.7258148193359375),
            osg::Vec3f(-54.722499847412109375, 54.722499847412109375, -60.052555084228515625),
            osg::Vec3f(-34.68780517578125, 34.68780517578125, -66.37929534912109375),
            osg::Vec3f(-14.6531162261962890625, 14.6531162261962890625, -72.70604705810546875),
            osg::Vec3f(5.3815765380859375, -5.3815765380859375, -75.35065460205078125),
            osg::Vec3f(25.41626739501953125, -25.41626739501953125, -67.96945953369140625),
            osg::Vec3f(45.450958251953125, -45.450958251953125, -60.58824920654296875),
            osg::Vec3f(65.48564910888671875, -65.48564910888671875, -53.20705413818359375),
            osg::Vec3f(85.5203399658203125, -85.5203399658203125, -45.825855255126953125),
            osg::Vec3f(105.55503082275390625, -105.55503082275390625, -38.44464874267578125),
            osg::Vec3f(125.5897216796875, -125.5897216796875, -31.063449859619140625),
            osg::Vec3f(145.6244049072265625, -145.6244049072265625, -23.6822509765625),
            osg::Vec3f(165.659088134765625, -165.659088134765625, -16.3010540008544921875),
            osg::Vec3f(185.6937713623046875, -185.6937713623046875, -8.91985416412353515625),
            osg::Vec3f(205.7284698486328125, -205.7284698486328125, -1.53864824771881103515625),
            osg::Vec3f(215, -215, 1.877177715301513671875),
        })) << mPath;
    }
}
