childEffectsCount = 1
childEffectsCost = {{0,0,0,1}}	-- 1 chaos
childEffectsIcon = {"ane"}
icon = "ane"

function getName()
	if language == "french" then
		return "Coup tellurique"
	else
		return "Telluric blast"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Coup tellurique"
	else
		return "Telluric blast"
	end
end

function getDescription()
	if language == "french" then
		return "Co�t d'activation : 1�. Inflige 2 d�g�ts � la cible. S'il s'agit d'une unit� d'alignement LOI, inflige 1 d�g�t suppl�mentaire."
	else
		return "Activation cost: 1�. Deals 2 damages on target unit. If its alignment is LAW, deals 1 extra damage."
	end
end

function onActivateEffect(effect)
	if effect == 1 then
		selectTarget("unit", "inrange")
	end
end

function onResolveChild(effect, params)
	if effect == 1 then
		player, unit = splitint(params, " ");
		life, law, death, chaos = getUnitAlignment(player, unit)
		if law == 1 then
			damageUnit(player, unit, 3)
		else
			damageUnit(player, unit, 2)
		end
	end
end
