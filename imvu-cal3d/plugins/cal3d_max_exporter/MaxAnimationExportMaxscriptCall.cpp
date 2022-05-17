//
// Copyright (C) 2004 Mekensleep
//
// Mekensleep
// 24 rue vieille du temple
// 75004 Paris
//       licensing@mekensleep.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

//----------------------------------------------------------------------------//
// Exporter.cpp                                                               //
//----------------------------------------------------------------------------//
// This program is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU General Public License as published by the Free //
// Software Foundation; either version 2 of the License, or (at your option)  //
// any later version.                                                         //
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
// Includes                                                                   //
//----------------------------------------------------------------------------//

#include <cal3d/coreanimation.h>
#include <cal3d/coretrack.h>
#include <cal3d/corekeyframe.h>
#include <cal3d/error.h>
#include <cal3d/saver.h>
#include "StdAfx.h"
#include "Exporter.h"
#include "BaseInterface.h"
#include "SkeletonCandidate.h"
#include "BoneCandidate.h"
#include "BaseNode.h"
#include "SkeletonExportSheet.h"
#include "AnimationExportSheet.h"
#include "MeshExportSheet.h"
#include "MaterialExportSheet.h"
#include "MeshCandidate.h"
#include "SubmeshCandidate.h"
#include "VertexCandidate.h"
#include "MaterialLibraryCandidate.h"
#include "MaterialCandidate.h"
#include "MaxAnimationExport.h"


#if defined(_UNICODE)
#define tstring wstring
#else
#define tstring string
#endif


bool CExporter::ExportAnimationFromMaxscriptCall(const std::string& strFilename, void* _AnimExportParams)
{
	if (!_AnimExportParams)
	{
		SetLastError("_AnimExportParams pointer is null.", __FILE__, __LINE__);
		return false;
	}

	AnimExportParams*	param = reinterpret_cast<AnimExportParams*>(_AnimExportParams);
	
	// check if a valid interface is set
	if(m_pInterface == 0)
	{
		SetLastError("m_pInterface == 0.", __FILE__, __LINE__);
		return false;
	}

	// build a skeleton candidate
	CSkeletonCandidate skeletonCandidate;

	//Remove user interface
	/*// show export wizard sheet
	CAnimationExportSheet sheet("Cal3D Animation Export", m_pInterface->GetMainWnd());
	sheet.SetSkeletonCandidate(&skeletonCandidate);
	sheet.SetAnimationTime(m_pInterface->GetStartFrame(), m_pInterface->GetEndFrame(), m_pInterface->GetCurrentFrame(), m_pInterface->GetFps());
	sheet.SetWizardMode();
	if(sheet.DoModal() != ID_WIZFINISH) return true;
	*/

	//Following block replaces the user interface interactions
	{
		// create the skeleton candidate from the skeleton file
		if(! skeletonCandidate.CreateFromSkeletonFile(param->m_skeletonfilepath))
		{
			MessageBoxA(0, "Skeleton Error", theExporter.GetLastError().c_str(), MB_OK | MB_ICONEXCLAMATION);
			return false;
		}	

		//Set all bones in our array of nodes selected
		const std::vector<CBoneCandidate *>& vectorBoneCandidate = skeletonCandidate.GetVectorBoneCandidate();

		int NumElemInTabMaxscript = param->m_tabbones.Count();

		// Select bone candidates that are in our array
		int idx = 0;
		const int numelems = vectorBoneCandidate.size();
		for (idx = 0;idx<numelems;idx++)
		{
			CBoneCandidate * bonecandidate = vectorBoneCandidate[idx];
			if (! bonecandidate)return false;

			//Deselect it
			bonecandidate->SetSelected(false);
			
			int j;
			for (j=0;j<NumElemInTabMaxscript;j++)
			{
				std::string bcname_		= bonecandidate->GetNode()->GetName();
                std::tstring bcname(bcname_.begin(), bcname_.end());
				std::tstring	bonename	= param->m_tabbones[j]->GetName();

				if (bcname == bonename)
				{
					//This bone candidate is in the array passed by maxscript, so select it.
					bonecandidate->SetSelected(true);
					break;
				}
			}
		}
	}

	// get the number of selected bone candidates
	int selectedCount;
	selectedCount = skeletonCandidate.GetSelectedCount();
	if(selectedCount == 0)
	{
		SetLastError("No bones selected to export.", __FILE__, __LINE__);
		return false;
	}

	// create the core animation instance
	CalCoreAnimation coreAnimation;

        // set the duration of the animation
	float duration;
	duration = (float)(param->m_endframe - param->m_startframe) / (float)m_pInterface->GetFps();
	coreAnimation.duration = duration;

	// get bone candidate vector
	const std::vector<CBoneCandidate *>& vectorBoneCandidate = skeletonCandidate.GetVectorBoneCandidate();

	size_t boneCandidateId;
	for(boneCandidateId = 0; boneCandidateId < vectorBoneCandidate.size(); boneCandidateId++)
	{
		// get the bone candidate
		CBoneCandidate* pBoneCandidate = vectorBoneCandidate[boneCandidateId];

		// only create tracks for the selected bone candidates
		if(pBoneCandidate->IsSelected())
		{
			coreAnimation.tracks.push_back(CalCoreTrack(boneCandidateId, CalCoreTrack::KeyframeList()));
		}
	}

  CStackProgress progress(m_pInterface, "Exporting to animation file...");

	// calculate the end frame
	int endFrame;
	endFrame = (int)(duration * (float)param->m_framerate + 0.5f);

	// calculate the displaced frame
  int displacedFrame;
  displacedFrame = (int)(((float)param->m_frameoffset / (float)m_pInterface->GetFps()) * (float)param->m_framerate + 0.5f) % endFrame;

	// calculate the possible wrap frame
  int wrapFrame;
  wrapFrame = (displacedFrame > 0) ? 1 : 0;
  float wrapTime;
  wrapTime = 0.0f;

  int frame;
  int outputFrame;
  for(frame = 0,  outputFrame = 0; frame <= (endFrame + wrapFrame); frame++)
	{
		// update the progress info
		m_pInterface->SetProgressInfo(int(100.0f * (float)frame / (float)(endFrame + wrapFrame + 1)));

		// calculate the time in seconds
		float time;
		time = (float)param->m_startframe / (float)m_pInterface->GetFps() + (float)displacedFrame / (float)param->m_framerate;

/* DEBUG
CString str;
str.Format("frame=%d, endframe=%d, disframe=%d, ouputFrame=%d (%f), time=%f\n", frame, endFrame, displacedFrame, outputFrame, (float)outputFrame / (float)param->m_framerate + wrapTime, time);
OutputDebugString(str);
*/

		for(boneCandidateId = 0; boneCandidateId < vectorBoneCandidate.size(); boneCandidateId++)
		{
			// get the bone candidate
			CBoneCandidate *pBoneCandidate;
			pBoneCandidate = vectorBoneCandidate[boneCandidateId];

			// only export keyframes for the selected bone candidates
			if(pBoneCandidate->IsSelected())
			{
				// allocate new core keyframe instance
				CalCoreKeyframe pCoreKeyframe;
				pCoreKeyframe.time = (float)outputFrame / (float)param->m_framerate + wrapTime;

				CalVector translation;
				CalQuaternion rotation;
				skeletonCandidate.GetTranslationAndRotation(boneCandidateId, time, translation, rotation);

				pCoreKeyframe.transform.translation = translation;
				pCoreKeyframe.transform.rotation = rotation;

                                // oh god
                                CalCoreTrack::KeyframeList& ls = const_cast<CalCoreTrack::KeyframeList&>(coreAnimation.getCoreTrack(pBoneCandidate->GetId())->keyframes);
                                ls.push_back(pCoreKeyframe);
			}
		}

    // calculate the next displaced frame and its frame time
    if(wrapFrame > 0)
    {
      if(displacedFrame == endFrame)
      {
        wrapTime = 0.0001f;
        displacedFrame = 0;
      }
      else
      {
        wrapTime = 0.0f;
        outputFrame++;
        displacedFrame++;
      }
    }
    else
    {
      outputFrame++;
      displacedFrame++;
   }
	}

	// save core animation to the file
	if(!CalSaver::saveCoreAnimation(strFilename, &coreAnimation))
	{
		SetLastError(CalError::getLastErrorText(), __FILE__, __LINE__);
		return false;
	}

	return true;
}
