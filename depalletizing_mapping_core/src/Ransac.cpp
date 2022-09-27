
#include "depalletizing_mapping_core/Ransac.hpp"

namespace depalletizing_mapping
{
    Ransac::Ransac()
    {
        PlaneModel planeModel;
        mModel = planeModel;
        float modelThreshold = 0.005f;
        mModel.SetModelThreshold(modelThreshold);
        mModelThreshold = modelThreshold;
    }

	Ransac::Ransac(PlaneModel& model, std::vector<Point3D>& data, float modelThreshold, int maxIteration)
		: mModel(model)
		, mData(data)
		, mMaxIteration(maxIteration)
	{
		mModel.SetModelThreshold(modelThreshold);
		mModelThreshold = modelThreshold;
	}

    Ransac::Ransac(PlaneModel& model, float modelThreshold, int maxIteration)
        : mModel(model)
        , mModelThreshold(modelThreshold)
        , mMaxIteration(maxIteration)
    {
        mModel.SetModelThreshold(modelThreshold);
        mModelThreshold = modelThreshold;
    }

    Ransac::Ransac(PlaneModel& model, float modelThreshold)
        : mModel(model)
        , mModelThreshold(modelThreshold)
    {
        mModel.SetModelThreshold(modelThreshold);
        mModelThreshold = modelThreshold;
    }

    Ransac::Ransac(float modelThreshold)
    {
        PlaneModel planeModel;
        mModel = planeModel;
        mModel.SetModelThreshold(modelThreshold);
        mModelThreshold = modelThreshold;
    }

	void Ransac::SetData(std::vector<Point3D>& data)
	{
		mData = std::move(data);
	}

    void Ransac::SetModelThreshold(float modelThreshold)
    {
        mModel.SetModelThreshold(modelThreshold);
    }

    void Ransac::SetMaxIteration (int maxiteration)
    {
        mMaxIteration = maxiteration;
    }

    int Ransac::GetMaxIteration() const
    {
        return mMaxIteration;
    }

    void Ransac::SetParams(float threshold, int upperPointsNum)
    {
        mModelThreshold = threshold;
        mUpperPointsNum = upperPointsNum;
    }

    std::vector<Point3D> Ransac::GetData() const
    {
        return mData;
    }

	std::vector<float> Ransac::GetBestModelParameters() const
	{
		return mBestModelParameters;
	}

	std::vector<PlaneModel> Ransac::GetResultModel() const
	{
		return mResultModel;
	}

	bool Ransac::bRun()
	{
		int iter = 0;
		while (iter != mMaxIteration)
		{
			std::vector<Point3D> randomPoints = getRandomPoints();
			mModel.FindParametersWithRandom(randomPoints);		    // mModel->mParameters에 들어가 있음.
			int inliearNum = getInlierNum();

			if (mInlierNum < inliearNum)	// update
			{
				mBestModelParameters = mModel.GetParameters();		// get mModel->mParameters
				mInlierNum = inliearNum;
			}
			iter++;
		}

		if (mBestModelParameters.empty())
		{
			return false;
		}

		return true;
	}

    void Ransac::RunMulti()
    {
        int upperPointsNum = mUpperPointsNum;

        std::vector<Point3D> upperPoints = getUpperPoints(upperPointsNum);

        mBestModelParameters.clear();
        mResultData.clear();

        for (int i = 0; i < upperPointsNum; i++)
        {
            std::vector<Point3D> randomPoints = getRandomPoints(upperPoints);
            mModel.FindParametersWithRandom(randomPoints);
            int inlierNum = getInlierNum(upperPoints);

            if (mInlierNum < inlierNum)
            {
                mBestModelParameters = mModel.GetParameters();
                mInlierNum = inlierNum;
            }
        }

        if (!mBestModelParameters.empty())
        {
            GetResult();
            if (!mResultData.empty())
            {
                PlaneModel bestModel(mResultData, mBestModelParameters);
                mResultModel.push_back(bestModel);
            }
        }
    }

	void Ransac::GetResult()
	{
		for (int i = 0; i < mData.size(); i++)
		{
			if (bIsContained(mData[i]))
			{
				mResultData.push_back(mData[i]);
			}
		}
	}

    void Ransac::GetResultMulti()
    {
        for (int i = 0; i < mData.size(); i++)
        {
            if (bIsContained(mData[i]))
            {
                mResultData.push_back(mData[i]);
                mData.erase(mData.begin() + i);
                i--;
            }
        }
    }

	std::vector<Point3D> Ransac::GetResultData() const
	{
		return mResultData;
	};

	std::vector<Point3D> Ransac::getUpperPoints(int number) const
	{
		std::vector<Point3D> upperPoints;
		for (int i = 0; i < number; i++)
		{
			upperPoints.push_back(mData[i]);
		}

		return upperPoints;
	}

		std::vector<Point3D> Ransac::getRandomPoints() const
        {
            int dataNum = mData.size();
            std::random_device randomDevice;
            std::mt19937 generator(randomDevice());
            std::uniform_int_distribution<> randomSample(0, dataNum);

            std::vector<Point3D> randomPoints;
            for (int i = 0; i < 3; i++)
            {
                int iter = randomSample(generator);
                randomPoints.push_back(mData[iter]);
            }
            return randomPoints;
        }

	std::vector<Point3D> Ransac::getRandomPoints(std::vector<Point3D>& upperPoints) const
	{
		int dataNum = upperPoints.size();
		std::random_device randomDevice;
		std::mt19937 generator(randomDevice());
		std::uniform_int_distribution<> randomSample(0, dataNum);

		std::vector<Point3D> randomPoints;
		for (int i = 0; i < 3; i++)
		{
			int iter = randomSample(generator);
			randomPoints.push_back(upperPoints[iter]);
		}

		return randomPoints;
	}

	int Ransac::getInlierNum()
	{
		int inlierNum = 0;
		for (int i = 0; i < mData.size(); i++)
		{
			if (mModel.bIsInThreshold(mData[i]))
			{
				inlierNum++;
			}
		}
		return inlierNum;
	}

	int Ransac::getInlierNum(std::vector<Point3D>& upperPoints)
	{
		int inlierNum = 0;
		for (int i = 0; i < upperPoints.size(); i++)
		{
			if (mModel.bIsInThreshold(upperPoints[i]))
			{
				inlierNum++;
			}
		}
		return inlierNum;
	}

	bool Ransac::bIsContained(Point3D& data)
	{
		float x = data.GetX();
		float y = data.GetY();
		float z = data.GetZ();

		float distance = std::abs(mBestModelParameters[0] * x + mBestModelParameters[1] * y + mBestModelParameters[2] * z - 1) / std::sqrt(mBestModelParameters[0] * mBestModelParameters[0] + mBestModelParameters[1] * mBestModelParameters[1] + mBestModelParameters[2] * mBestModelParameters[2]);

		if (distance < mModelThreshold)
		{
			return true;
		}
		return false;
	}

	bool Ransac::bIsContainedMulit(Point3D& data, std::vector<float>& parameters)
	{
		float x = data.GetX();
		float y = data.GetY();
		float z = data.GetZ();

		float distance = std::abs(mBestModelParameters[0] * x + mBestModelParameters[1] * y + mBestModelParameters[2] * z - 1) / std::sqrt(mBestModelParameters[0] * mBestModelParameters[0] + mBestModelParameters[1] * mBestModelParameters[1] + mBestModelParameters[2] * mBestModelParameters[2]);

		if (distance < mModelThreshold)
		{
			return true;
		}
		return false;
	}
}