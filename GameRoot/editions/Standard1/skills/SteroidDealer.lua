childEffectsCount = 1
childEffectsCost = {{0,0,0,1}}	-- 1 chaos
childEffectsIcon = {"ane"}
icon = "ane"

function getName()
	if language == "french" then
		return "Dealer de stéroïdes"
	else
		return "Steroids dealer"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Stéroïdes"
	else
		return "Steroids"
	end
end

function getDescription()
	if language == "french" then
		return "Coût d'activation : 1§. La mêlée de l'unité ciblée augmente de 2 ce tour-ci."
	else
		return "Activation cost: 1§. Melee of target unit is increased by 2 this turn."
	end
end

function onActivateEffect(effect)
	if effect == 1 then
		selectTarget("unit", "inrange")
	end
end

nbSteroids = 0
steroidLifetime = 1
currentSteroids = {}

-- this function receives playedId and unitId as parameter, in a space-separated string
function onResolveChild(effect, params)
	if effect == 1 then
		player, unit = split(params, " ")
		addChildEffectToUnit(1, player, unit)
		-- store active steroids to remove them later
		nbSteroids = nbSteroids + 1
		currentSteroids[nbSteroids] = {player, unit, 0}
	end
end

function child_getMod_melee(effect, value)
	if effect == 1 then
		return value + 2
	end
	return value
end

function onNewUnitTurn()
	-- increase turn counter of every active steroid
	i = 1
	while i <= nbSteroids do
		currentSteroids[i][3] = currentSteroids[i][3] + 1
		-- remove if too old
		if currentSteroids[i][3] > steroidLifetime then
			removeChildEffectFromUnit(1, currentSteroids[i][1], currentSteroids[i][2])
			table.remove(currentSteroids, i)
			nbSteroids = nbSteroids - 1
			i = i - 1
		end
		i = i + 1
	end
end
