#include "Features.h"

void CBulletTracer::Log(IGameEvent* pEvent)
{
	if (strstr(pEvent->GetName(), str("bullet_impact")))
	{
		int iUser = interfaces.engine->GetPlayerForUserID(pEvent->GetInt(str("userid")));

		auto pPlayer = interfaces.ent_list->GetClientEntity(iUser);

		if (!pPlayer)
			return;


		if (!pPlayer->IsPlayer())
			return;


		if (pPlayer->GetTeam() == csgo->local->GetTeam() && pPlayer != csgo->local)
			return;

		if (pPlayer->IsDormant())
			return;

		float x, y, z;
		x = pEvent->GetFloat(str("x"));
		y = pEvent->GetFloat(str("y"));
		z = pEvent->GetFloat(str("z"));

		if (pPlayer == csgo->local)
		{
			if (vars.visuals.bullet_impact)
			{
				const auto& size = vars.visuals.impacts_size / 10.f;
				interfaces.debug_overlay->add_box_overlay(Vector(x, y, z),
					Vector(-size, -size, -size),
					Vector(size, size, size), Vector(0, 0, 0),
					vars.visuals.bullet_impact_color[0],
					vars.visuals.bullet_impact_color[1],
					vars.visuals.bullet_impact_color[2], vars.visuals.bullet_impact_color[3], 4.f);
			}
		}
		
		if (vars.visuals.bullet_tracer || vars.visuals.bullet_tracer_local)
		{
			data[pPlayer->GetIndex()].push_back(Vector(x, y, z));
		}
	}
}

void CBulletTracer::Proceed()
{
	for (auto i = 0; i < 64; i++) {
		auto& a = data[i];
		if (a.size() > 0) {
			auto player = interfaces.ent_list->GetClientEntity(i);
			if (player)
				Add(player, player == csgo->local ? csgo->last_shoot_pos : player->GetEyePosition(), a.back(),
					player == csgo->local ? vars.visuals.bullet_tracer_local_color : vars.visuals.bullet_tracer_color);
			
			a.clear();
		}
	}
}

void CBulletTracer::Draw()
{
	if (!interfaces.engine->IsInGame())
	{
		tracers.clear();
		return;
	}

	std::string model_name = str("sprites/");
	switch (vars.visuals.bullet_tracer_type)
	{
	case 0:
		model_name += str("purplelaser1.vmt");
		break;
	case 1:
		model_name += str("physbeam.vmt");
		break;
	case 2:
		model_name += str("bubble.vmt");
		break;
	case 3:
		model_name += str("glow01.vmt");
		break;
	}

	for (int i = 0; i < tracers.size(); i++)
	{
		auto& current = tracers[i];
		bool is_local = current.player == csgo->local;
		bool check = (vars.visuals.bullet_tracer && !is_local) || (vars.visuals.bullet_tracer_local && is_local);
		if (!check)
			continue;
		BeamInfo_t beamInfo;
		beamInfo.m_nType = TE_BEAMPOINTS;
		beamInfo.m_pszModelName = model_name.c_str();
		beamInfo.m_nModelIndex = -1;
		beamInfo.m_flHaloScale = 0.0f;
		beamInfo.m_flLife = 5.f; //duration of tracers
		beamInfo.m_flWidth = 2; //start width
		beamInfo.m_flEndWidth = 2; //end width
		beamInfo.m_flFadeLength = 0.0f;
		beamInfo.m_flAmplitude = 2.0f;
		beamInfo.m_flBrightness = current.color.get_alpha();
		beamInfo.m_flSpeed = 0.2f;
		beamInfo.m_nStartFrame = 0;
		beamInfo.m_flFrameRate = 0.f;
		beamInfo.m_flRed = current.color.get_red();
		beamInfo.m_flGreen = current.color.get_green();
		beamInfo.m_flBlue = current.color.get_blue();
		beamInfo.m_nSegments = 2;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;
		beamInfo.m_vecStart = current.src;
		beamInfo.m_vecEnd = current.dst;

		Beam_t* beam = interfaces.beams->CreateBeamPoints(beamInfo);
		if (beam)
			interfaces.beams->DrawBeam(beam);

		tracers.erase(tracers.begin() + i);
	}
}